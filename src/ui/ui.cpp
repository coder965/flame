#include <process.h>
#include <map>

#include "../global.h"
#include "../input.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/renderpass.h"
#include "../graphics/descriptor.h"
#include "../graphics/pipeline.h"
#include "../graphics/sampler.h"
#include "../graphics/command_buffer.h"
#include "../application.h"

#include "ui.h"

namespace ImGui
{
	bool main_menu_alive = false;
	bool last_frame_main_menu_alive = false;

	bool BeginMenu_keepalive(const char* label, bool enabled)
	{
		auto open = BeginMenu(label, enabled);
		if (!main_menu_alive && open)
			main_menu_alive = true;
		return open;
	}

	bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size2));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return SplitterBehavior(id, bb, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}
}

namespace tke
{
	static Pipeline *pipeline_ui;
	static CommandBuffer *cb_ui;
	static std::unique_ptr<ImmediateVertexBuffer> vertexBuffer_ui;
	static std::unique_ptr<ImmediateIndexBuffer> indexBuffer_ui;

	static Image *fontImage;
	void initUi()
	{
		{
			struct Vertex2D
			{
				glm::vec2       pos;
				glm::vec2       uv;
				std::uint32_t   col;
			};

			static VkPipelineVertexInputStateCreateInfo vis;

			static VkVertexInputBindingDescription bindings = {0, sizeof(Vertex2D), VK_VERTEX_INPUT_RATE_VERTEX};

			static VkVertexInputAttributeDescription attributes[] = {
				{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, pos)},
				{1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, uv)},
				{2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(Vertex2D, col)}
			};

			vis = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);

			pipeline_ui = new Pipeline(PipelineCreateInfo()
				.vertex_input(&vis)
				.cullMode(VK_CULL_MODE_NONE)
				.addBlendAttachmentState(true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
				.addShader(engine_path + "shader/ui.vert", {})
				.addShader(engine_path + "shader/ui.frag", {}),
				renderPass_window, 0, true);
		}

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
				fontImage = new Image(width, height, VK_FORMAT_R8G8B8A8_UNORM,
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
				fontImage->fillData(0, pixels, width * height * 4);
				io.Fonts->TexID = (void*)0; // image index

				updateDescriptorSets(1, &pipeline_ui->descriptorSet->imageWrite(0, 0, fontImage, colorSampler));
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

	void ui_onKeyDown(int k)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[k] = true;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void ui_onKeyUp(int k)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[k] = false;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void ui_onChar(int c)
	{
		if (c == VK_TAB)
			return;

		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter((unsigned short)c);
	}

	static std::map<Image*, int> _images_map;
	static std::pair<std::shared_ptr<Image>, int> _images[127];

	static bool need_clear = false;
	void beginUi(bool _need_clear)
	{
		static int last_time = 0;
		if (last_time == 0) last_time = nowTime;

		need_clear = _need_clear;

		uiAcceptedMouse = false;
		uiAcceptedKey = false;

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

		ImGui::main_menu_alive = false;

		for (int i = 0; i < TK_ARRAYSIZE(_images); i++)
			_images[i].second = 0;
	}

	void endUi()
	{
		{
			std::vector<VkWriteDescriptorSet> writes;
			for (int i = 0; i < TK_ARRAYSIZE(_images); i++)
			{
				if (_images[i].first)
				{
					if (_images[i].second == 0)
					{
						_images_map.erase(_images[i].first.get());
						_images[i].first.reset();
						writes.push_back(pipeline_ui->descriptorSet->imageWrite(0, i + 1, fontImage, colorSampler));
					}
					else if (_images[i].second == 2)
						writes.push_back(pipeline_ui->descriptorSet->imageWrite(0, i + 1, _images[i].first.get(), colorSampler));
				}
			}
			updateDescriptorSets(writes.size(), writes.data());
		}

		ImGui::last_frame_main_menu_alive = ImGui::main_menu_alive;

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

			VkClearValue clear_value = {bkColor.r, bkColor.g, bkColor.b};
			cb_ui->beginRenderPass(need_clear ? renderPass_windowC : renderPass_window,
				window_framebuffers[window_imageIndex].get(), need_clear ? &clear_value : nullptr);

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

			addCb(cb_ui->v);
		}

		uiAcceptedMouse = ImGui::IsMouseHoveringAnyWindow();
		uiAcceptedKey = ImGui::IsAnyItemActive();
	}

	int get_ui_image_index(std::shared_ptr<Image> img)
	{
		auto it = _images_map.find(img.get());
		if (it != _images_map.end())
		{
			_images[it->second].second = 1;
			return it->second + 1;
		}
		for (int i = 0; i < TK_ARRAYSIZE(_images); i++)
		{
			if (!_images[i].first)
			{
				_images[i].first = img;
				_images[i].second = 2;
				_images_map[img.get()] = i;
				return i + 1;
			}
		}
		return 0;
	}
}
