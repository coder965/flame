#include <process.h>
#include <map>

#include "../global.h"
#include "../spare_list.h"
#include "../input.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/renderpass.h"
#include "../graphics/descriptor.h"
#include "../graphics/pipeline.h"
#include "../graphics/sampler.h"
#include "../graphics/command_buffer.h"
#include "../engine.h"

#include "ui.h"

const unsigned int ImageCount = 127;

static tke::SpareList _image_list(ImageCount);
static std::pair<std::shared_ptr<tke::Image>, tke::Op> _image_ops[ImageCount];

namespace ImGui
{
	ImVec2 SplitterThickness = ImVec2(8.f, 4.f);

	void TextVFilted(const char* fmt, const char* filter, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
		if (filter[0] && !strstr(g.TempBuffer, filter))
			return;
		TextUnformatted(g.TempBuffer, text_end);
	}

	bool Splitter(bool split_vertically, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
	{
		if (split_vertically)
			*size2 = GetWindowWidth() - GetStyle().WindowPadding.x * 2.f - *size1 - SplitterThickness.x;
		else
			*size2 = GetWindowHeight() - GetStyle().WindowPadding.x * 2.f - *size1 - SplitterThickness.y;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(SplitterThickness.x, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, SplitterThickness.y), 0.0f, 0.0f);
		return SplitterBehavior(id, bb, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

	ImTextureID ImageID(std::shared_ptr<tke::Image> i)
	{
		auto index = _image_list.add(i.get());
		if (index == -2)
		{
			index = i->ui_index;
			if (_image_ops[index].second == tke::OpNeedRemove)
				_image_ops[index].second = tke::OpKeep;
			return ImTextureID(index + 1);
		}
		else if (index == -1)
		{
			i->ui_index = -1;
			return 0;
		}
		i->ui_index = index;
		_image_ops[index].first = i;
		_image_ops[index].second = tke::OpNeedUpdate;
		return ImTextureID(index + 1);
	}

	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		if (border_col.w > 0.0f)
			bb.Max += ImVec2(2, 2);
		ItemSize(bb);
		if (!ItemAdd(bb, 0))
			return;

		if (border_col.w > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
			bb.Expand(-1);
		}

		if (filename != "")
		{
			auto i = tke::get_image(filename);
			if (i)
				window->DrawList->AddImage(ImageID(i), bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), GetColorU32(ImVec4(1, 1, 1, 1)));
		}
	}

	bool BeginStatusBar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		auto height = ImGui::GetTextLineHeight() + ImGui::GetStyle().WindowPadding.y * 2.f;
		ImGui::SetNextWindowPos(ImVec2(0, tke::window_cy - height));
		ImGui::SetNextWindowSize(ImVec2(tke::window_cx, height));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.11f, 0.14f, 1.f));
		return ImGui::Begin("status", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
	}

	void EndStatusBar()
	{
		ImGui::End();
		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar(1);
	}
}

namespace tke
{
	namespace ui
	{
		bool accepted_mouse;
		bool accepted_key;

		static std::list<std::unique_ptr<Window>> windows;

		Window::Window(const std::string &_title, bool _enable_menu, bool _enable_saved_settings, bool _modal) :
			first(true),
			first_cx(0),
			first_cy(0),
			title(_title),
			enable_menu(_enable_menu),
			enable_saved_settings(_enable_saved_settings),
			modal(_modal),
			opened(true),
			_need_focus(false),
			layout(nullptr)
		{
			windows.emplace_back(this);
		}

		void Window::show()
		{
			if (_need_focus)
			{
				ImGui::SetNextWindowFocus();
				_need_focus = false;
			}
			if (first)
			{
				if (first_cx != 0 && first_cy != 0)
					ImGui::SetNextWindowSize(ImVec2(first_cx, first_cy));
			}

			ImGuiWindowFlags _flags;
			if (enable_menu)
				_flags |= ImGuiWindowFlags_MenuBar;
			if (!enable_saved_settings)
				_flags |= ImGuiWindowFlags_NoSavedSettings;
			bool _open = true;
			if (modal)
			{
				if (first)
					ImGui::OpenPopup(title.c_str());
				_open = ImGui::BeginPopupModal(title.c_str(), &opened);
			}
			else
			{
				if (!layout)
					_open = ImGui::Begin(title.c_str(), &opened);
			}
			if (_open)
			{
				on_show();
				if (modal)
					ImGui::EndPopup();
			}
			if (!modal && !layout)
				ImGui::End();

			first = false;
		}

		Layout::Layout() :
			mode(ModeNull)
		{
			size[0] = 100.f;
			windows[0] = nullptr;
			windows[1] = nullptr;
		}

		void Layout::show()
		{
			switch (mode)
			{
				case ModeCenter:
					windows[0]->show();
					break;
				case ModeHorizontal: case ModeVertival:
				{
					ImGui::Splitter(mode == ModeHorizontal, &size[0], &size[1], 50.f, 50.f);
					ImGui::PushID(this);
					ImGui::BeginChild("##part1", mode == ModeHorizontal ? ImVec2(size[0], 0) : ImVec2(0, size[0]), true);
					if (children[0])
						children[0]->show();
					else
						windows[0]->show();
					ImGui::EndChild();
					if (mode == ModeHorizontal)
						ImGui::SameLine();
					ImGui::BeginChild("##part2", ImVec2(0, 0), true);
					if (children[1])
						children[1]->show();
					else
						windows[1]->show();
					ImGui::EndChild();
					ImGui::PopID();
					break;
				}
			}
		}

		void Layout::add_window(Window *w, int idx/*left or right, top or bottom*/, DockDirection dir)
		{
			if (mode == ModeNull)
			{
				mode = ModeCenter;
				windows[0] = w;
				w->layout = this;
				return;
			}
			if (mode == ModeCenter)
			{
				switch (dir)
				{
					case DockCenter:
						assert(0); // WIP
						break;
					case DockLeft:
						mode = ModeHorizontal;
						windows[1] = windows[0];
						windows[0] = w;
						break;
					case DockRight:
						mode = ModeHorizontal;
						windows[1] = w;
						break;
					case DockTop:
						mode = ModeVertival;
						windows[1] = windows[0];
						windows[0] = w;
						break;
					case DockBottom:
						mode = ModeVertival;
						windows[1] = w;
						break;
				}
				w->layout = this;
				return;
			}
			switch (dir)
			{
				case DockCenter:
					assert(0); // WIP
					break;
				case DockLeft:
					assert(0); // WIP
					break;
				case DockRight:
					switch (mode)
					{
						case ModeHorizontal:
						{
							switch (idx)
							{
								case 0:
								{
									assert(0); // WIP
									break;
								}
								case 1:
								{
									auto l = new Layout;
									l->mode = ModeHorizontal;
									l->windows[1] = w;
									w->layout = l;
									if (children[0])
										l->children[0] = std::move(children[1]);
									else
									{
										l->windows[0] = windows[1];
										windows[1]->layout = l;
									}
									children[1] = std::unique_ptr<Layout>(l);
									windows[1] = nullptr;
									break;
								}
							}
							break;
						}
						case ModeVertival:
							switch (idx)
							{
								case 0:
								{
									auto l = new Layout;
									l->mode = ModeHorizontal;
									l->windows[1] = w;
									w->layout = l;
									if (children[0])
										l->children[0] = std::move(children[0]);
									else
									{
										l->windows[0] = windows[1];
										windows[1]->layout = l;
									}
									children[0] = std::unique_ptr<Layout>(l);
									windows[0] = nullptr;
									break;
								}
								case 1:
								{
									assert(0); // WIP
									break;
								}
							}
							break;
					}
					break;
				case DockTop:
					assert(0); // WIP
					break;
				case DockBottom:
					assert(0); // WIP
					break;
			}
		}

		Layout main_layout;

		glm::vec4 bg_color = glm::vec4(0.35f, 0.57f, 0.1f, 1.f);

		glm::vec4 get_bg_color()
		{
			return bg_color;
		}

		void set_bg_color(const glm::vec4 &v)
		{
			bg_color = v;
		}

		static Pipeline *pipeline_ui;
		static CommandBuffer *cb_ui;
		static std::unique_ptr<ImmediateVertexBuffer> vertexBuffer_ui;
		static std::unique_ptr<ImmediateIndexBuffer> indexBuffer_ui;

		static Image *font_image;
		void init()
		{
			add_keydown_listener([](int k) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[k] = true;

				io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
				io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
				io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
				io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
			});

			add_keyup_listener([](int k) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[k] = false;

				io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
				io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
				io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
				io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
			});

			add_char_listener([](int c) {
				if (c == VK_TAB)
					return;

				ImGuiIO& io = ImGui::GetIO();
				if (c > 0 && c < 0x10000)
					io.AddInputCharacter((unsigned short)c);
			});

			pipeline_ui = new Pipeline(PipelineCreateInfo()
				.vertex_input_state({ { TokenF32V2, 0 },{ TokenF32V2, 0 },{ TokenB8V4, 0 } })
				.cull_mode(VK_CULL_MODE_NONE)
				.add_blend_attachment_state(true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
				.add_shader(engine_path + "shader/ui.vert", {})
				.add_shader(engine_path + "shader/ui.frag", {}),
				renderPass_window, 0, true);

			{
				ImGuiIO& io = ImGui::GetIO();
				{
					//io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msmincho.ttc", 16, nullptr, io.Fonts->GetGlyphRangesJapanese());
					io.Fonts->AddFontDefault();
					static const ImWchar icons_ranges[] = {
						ICON_MIN_FA,
						ICON_MAX_FA,
						0
					};
					ImFontConfig icons_config;
					icons_config.MergeMode = true;
					icons_config.PixelSnapH = true;
					io.Fonts->AddFontFromFileTTF("icon.ttf", 16.0f, &icons_config, icons_ranges);
					unsigned char* pixels; int width, height;
					io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
					font_image = new Image(width, height, VK_FORMAT_R8G8B8A8_UNORM,
						VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
					font_image->fillData(0, pixels, width * height * 4);
					io.Fonts->TexID = (void*)0; // image index

					updateDescriptorSets(1, &pipeline_ui->descriptorSet->imageWrite(0, 0, font_image, colorSampler));
				}

				cb_ui = new CommandBuffer;
				cb_ui->begin();
				cb_ui->end();

				io.KeyMap[ImGuiKey_Tab] = VK_TAB;
				io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
				io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
				io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
				io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
				io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
				io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
				io.KeyMap[ImGuiKey_Home] = VK_HOME;
				io.KeyMap[ImGuiKey_End] = VK_END;
				io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
				io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
				io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
				io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
				io.KeyMap[ImGuiKey_A] = 'A';
				io.KeyMap[ImGuiKey_C] = 'C';
				io.KeyMap[ImGuiKey_V] = 'V';
				io.KeyMap[ImGuiKey_X] = 'X';
				io.KeyMap[ImGuiKey_Y] = 'Y';
				io.KeyMap[ImGuiKey_Z] = 'Z';
				io.SetClipboardTextFn = [](void *user_data, const char *s) {
					set_clipBoard(s);
				};
				io.GetClipboardTextFn = [](void *user_data) {
					static std::string s;
					s = get_clipBoard();
					return s.c_str();
				};

				ImGuiStyle& style = ImGui::GetStyle();
				style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
				style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 0.50f);
				style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 0.50f);
			}
		}

		void begin()
		{
			static int last_time = 0;
			if (last_time == 0) last_time = nowTime;

			accepted_mouse = false;
			accepted_key = false;

			ImGuiIO& io = ImGui::GetIO();

			io.DisplaySize = ImVec2((float)window_cx, (float)window_cy);
			io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			io.DeltaTime = (float)((nowTime - last_time) / 1000.f);
			last_time = nowTime;

			io.MousePos = ImVec2((float)mouseX, (float)mouseY);

			io.MouseDown[0] = mouseLeft.pressing;
			io.MouseDown[1] = mouseRight.pressing;
			io.MouseDown[2] = mouseMiddle.pressing;

			io.MouseWheel = mouseScroll / 120;

			ImGui::NewFrame();

			_image_list.iterate([&](int index, void *p, bool &remove) {
				_image_ops[index].second = OpNeedRemove;
				return true;
			});
		}

		void end()
		{
			if (main_layout.mode != Layout::ModeNull)
			{
				auto text_height = ImGui::GetTextLineHeight();
				auto menubar_height = text_height + ImGui::GetStyle().FramePadding.y * 2.f;
				ImGui::SetNextWindowPos(ImVec2(0.f, menubar_height));
				ImGui::SetNextWindowSize(ImVec2(window_cx, window_cy - menubar_height - text_height - ImGui::GetStyle().WindowPadding.y * 2.f));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
				ImGui::Begin("##dock", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
					ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
				main_layout.show();
				ImGui::End();
				ImGui::PopStyleVar();
			}

			for (auto it = windows.begin(); it != windows.end(); )
			{
				if (!(*it)->opened)
					it = windows.erase(it);
				else
					it++;
			}

			for (auto &w : windows)
			{
				if (!w->layout)
					w->show();
			}

			{
				std::vector<VkWriteDescriptorSet> writes;
				_image_list.iterate([&](int index, void *p, bool &remove) {
					auto op = _image_ops[index].second;
					if (op == OpNeedRemove)
					{
						remove = true;
						_image_ops[index].first.reset();
						writes.push_back(pipeline_ui->descriptorSet->imageWrite(0, index + 1, font_image, colorSampler));
					}
					else if (op == OpNeedUpdate)
					{
						auto image = (Image*)p;
						writes.push_back(pipeline_ui->descriptorSet->imageWrite(0, index + 1, image, colorSampler));
					}
					return true;
				});
				updateDescriptorSets(writes.size(), writes.data());
			}

			ImGui::Render();

			ImGuiIO& io = ImGui::GetIO();
			if ((int)(io.DisplaySize.x * io.DisplayFramebufferScale.x) > 0 && (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y) > 0)
			{
				auto draw_data = ImGui::GetDrawData();
				if (draw_data->CmdListsCount > 0)
				{
					draw_data->ScaleClipRects(io.DisplayFramebufferScale);

					size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
					if (!vertexBuffer_ui || vertexBuffer_ui->size < vertex_size)
						vertexBuffer_ui = std::make_unique<ImmediateVertexBuffer>(vertex_size);

					size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
					if (!indexBuffer_ui || indexBuffer_ui->size < index_size)
						indexBuffer_ui = std::make_unique<ImmediateIndexBuffer>(index_size);

					auto vtx_dst = (ImDrawVert*)vertexBuffer_ui->map(0, vertex_size);
					auto idx_dst = (ImDrawIdx*)indexBuffer_ui->map(0, index_size);
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
						memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

						vtx_dst += cmd_list->VtxBuffer.Size;
						idx_dst += cmd_list->IdxBuffer.Size;
					}
					vertexBuffer_ui->unmap();
					indexBuffer_ui->unmap();
				}

				cb_ui->reset();
				cb_ui->begin();

				if (bg_color.a > 0.f)
				{
					VkClearValue clear_value = { bg_color.r, bg_color.g, bg_color.b, 1.f };
					cb_ui->beginRenderPass(renderPass_windowC, window_framebuffers[window_imageIndex].get(), &clear_value);
				}
				else
					cb_ui->beginRenderPass(renderPass_window,
						window_framebuffers[window_imageIndex].get());

				if (draw_data->CmdListsCount > 0)
				{
					cb_ui->setViewportAndScissor(window_cx, window_cy);

					cb_ui->bindVertexBuffer(vertexBuffer_ui.get());
					cb_ui->bindIndexBuffer(indexBuffer_ui.get(), VK_INDEX_TYPE_UINT16);

					cb_ui->bindPipeline(pipeline_ui);
					cb_ui->bindDescriptorSet();

					cb_ui->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &glm::vec4(2.f / io.DisplaySize.x, 2.f / io.DisplaySize.y, -1.f, -1.f));

					int vtx_offset = 0;
					int idx_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
							if (pcmd->UserCallback)
							{
								pcmd->UserCallback(cmd_list, pcmd);
								pcmd->TextureId;
							}
							else
							{
								cb_ui->setScissor(ImMax((int32_t)(pcmd->ClipRect.x), 0),
									ImMax((int32_t)(pcmd->ClipRect.y), 0),
									ImMax((uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), 0),
									ImMax((uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1), 0)); // TODO: + 1??????
								cb_ui->drawIndex(pcmd->ElemCount, idx_offset, vtx_offset, 1, (int)pcmd->TextureId);
							}
							idx_offset += pcmd->ElemCount;
						}
						vtx_offset += cmd_list->VtxBuffer.Size;
					}
				}

				cb_ui->endRenderPass();

				cb_ui->end();

				add_to_drawlist(cb_ui->v);
			}

			accepted_mouse = ImGui::IsMouseHoveringAnyWindow();
			accepted_key = ImGui::IsAnyItemActive();
		}
	}
}
