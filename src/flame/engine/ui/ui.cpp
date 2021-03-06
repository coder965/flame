#include <process.h>
#include <map>

#include <flame/filesystem.h>
#include <flame/spare_list.h>
#include <flame/system.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/pipeline.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/ui/ui.h>
#include <flame/engine/ui/layout.h>
#include <flame/engine/ui/window.h>

const unsigned int ImageCount = 127;

static flame::SpareList _image_list(ImageCount);

namespace ImGui
{
	const float SplitterThickness = 4.f;

	Splitter::Splitter(bool _vertically, float _min_size1, float _min_size2) :
		vertically(_vertically)
	{
		min_size[0] = size[0] = _min_size1;
		min_size[1] = size[1] = _min_size2;
		set_general_draw_offset();
	}

	void Splitter::set_size_greedily()
	{
		if (vertically)
			size[1] = GetWindowWidth() - GetStyle().ItemSpacing.x - size[0];
		else
			size[1] = GetWindowHeight() - GetStyle().ItemSpacing.y - size[0];
	}

	void Splitter::set_general_draw_offset()
	{
		draw_offset = ((vertically ? ImGui::GetStyle().ItemSpacing.x : ImGui::GetStyle().ItemSpacing.y) - ImGui::SplitterThickness) / 2.f;
	}

	void Splitter::set_vertically(bool _vertically)
	{
		vertically = _vertically;
		set_general_draw_offset();
	}

	bool Splitter::do_split()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (vertically ? ImVec2(size[0] + draw_offset, 0.0f) : ImVec2(0.0f, size[0] + draw_offset));
		bb.Max = bb.Min + CalcItemSize(vertically ? ImVec2(SplitterThickness, -1) : ImVec2(-1, SplitterThickness), 0.0f, 0.0f);
		auto col = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
		col.w = 0.f;
		PushStyleColor(ImGuiCol_Separator, col);
		auto ret = SplitterBehavior(id, bb, vertically ? ImGuiAxis_X : ImGuiAxis_Y, &size[0], &size[1], min_size[0], min_size[1], 0.0f);
		PopStyleColor();
		return ret;
	}

	float menubar_height;
	float toolbar_height;
	float statusbar_height;

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

	void Image_s(flame::Texture *t, const ImVec2& size, const ImVec4& border_col)
	{
		Image(ImTextureID(t->ui_index), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), border_col);
	}

	bool ImageButton_s(flame::Texture *t, const ImVec2& size, bool active)
	{
		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (active)
			PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
		auto pressed = ImageButton(ImTextureID(t->ui_index), size, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
		if (active)
			PopStyleColor();
		PopStyleColor();
		return pressed;
	}

	bool IconButton(const char *label, float font_scale)
	{
		if (font_scale != 1.f)
			SetWindowFontScale(font_scale);
		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto pressed = Button(label);
		PopStyleColor();
		if (font_scale != 1.f)
			SetWindowFontScale(1.f);
		return pressed;
	}

	bool Checkbox_2in1(const char *label, bool *v)
	{
		auto window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImVec2 pos = GetCursorScreenPos();
		ImDrawList* draw_list = GetWindowDrawList();

		ImVec2 label_size = CalcTextSize(label, NULL, true);
		label_size.x += 8;

		auto pressed = InvisibleButton(label, label_size);
		if (pressed)
			*v = !*v;

		float t = *v ? 1.0f : 0.0f;

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(*v ? ImVec4(0.64f, 0.83f, 0.34f, 1.0f) : ImVec4(0.78f, 0.78f, 0.78f, 1.0f));
		else
			col_bg = ImGui::GetColorU32(*v ? ImVec4(0.56f, 0.83f, 0.26f, 1.0f) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));

		draw_list->AddRectFilled(pos, pos + label_size, col_bg, 4);
		RenderText(pos + ImVec2(4, 0), label);

		return pressed;
	}

	bool BeginToolBar()
	{
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		SetNextWindowPos(ImVec2(0, menubar_height));
		SetNextWindowSize(ImVec2(flame::surface->cx, toolbar_height));
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.91f, 0.94f, 1.f));
		return Begin("toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
	}

	void EndToolBar()
	{
		End();
		PopStyleColor();
		PopStyleVar();
	}

	static int _statusbar_int_debug;

	bool BeginStatusBar()
	{
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		SetNextWindowPos(ImVec2(0, flame::surface->cy - statusbar_height));
		SetNextWindowSize(ImVec2(flame::surface->cx, statusbar_height));
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.91f, 0.94f, 1.f));
		auto open = ImGui::Begin("statusbar", nullptr, ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
		Text("%d", _statusbar_int_debug);
		SameLine();
		return open;
	}

	void EndStatusBar()
	{
		End();
		PopStyleColor();
		PopStyleVar();
	}

	void BeginOverlapWindow(const char *title)
	{
		SetNextWindowPos(ImVec2(0, 0));
		SetNextWindowSize(ImVec2(flame::surface->cx, flame::surface->cy));
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		Begin(title, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
	}

	void EndOverlapWindow()
	{
		End();
		PopStyleVar();
	}
}

namespace flame
{
	namespace ui
	{
		bool accepted_mouse;
		bool accepted_key;

		glm::vec4 bg_color = glm::vec4(0.35f, 0.57f, 0.1f, 1.f);

		glm::vec4 get_bg_color()
		{
			return bg_color;
		}

		void set_bg_color(const glm::vec4 &v)
		{
			bg_color = v;
		}

		static std::shared_ptr<Framebuffer> framebuffers[2];

		static void on_resize(int, int)
		{
			for (auto i = 0; i < 2; i++)
				framebuffers[i] = get_framebuffer(surface->cx, surface->cy, renderpass.get(), 1, &surface->image_views[i]->v);

			resize_layout();
		}

		static CommandBuffer *cmds;
		static std::unique_ptr<Buffer> vertexBuffer_ui;
		static std::unique_ptr<Buffer> indexBuffer_ui;

		static Texture *font_image;

		static const char *sdf_chars = "\" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\"";
		static int sdf_char_count = strlen(sdf_chars) - 2;
		static int sdf_size = 32;

		struct SdfDrawCommand
		{
			std::string text;
			int x;
			int y;
			int size;
		};
		static std::vector<SdfDrawCommand> sdf_draw_commands;

		struct SdfVertex
		{
			glm::vec2 pos;
			glm::vec2 uv;
		};
		static std::unique_ptr<Buffer> sdf_vertex_buffer;

		static Texture *sdf_font_image;

		static Pipeline *pipeline_sdf;

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

			pipeline_sdf = new Pipeline(PipelineInfo()
				.set_vertex_input_state({{ TokenF32V2, 0 },{ TokenF32V2, 0 }})
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_blend_attachment_state(true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.add_shader("sdf_text.vert", {})
				.add_shader("sdf_text.frag", {}),
				renderpass.get(), 0, true);

			//io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msmincho.ttc", 16, nullptr, io.Fonts->GetGlyphRangesJapanese());

			static const ImWchar icons_ranges[] = {
				ICON_MIN_FA,
				ICON_MAX_FA,
				0
			};
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			io.Fonts->AddFontFromFileTTF("icon.ttf", 16.0f, &icons_config, icons_ranges);

			if (!std::filesystem::exists("sdf.rimg"))
			{
				std::string cl(sdf_chars);
				cl += " ";
				cl += std::to_string(sdf_size);
				exec("sdf_generator", cl.c_str());
			}

			{
				auto sdf_image = load_image("sdf.rimg");
				sdf_image->add_alpha_channel();
				sdf_font_image = new Texture(TextureTypeImage, sdf_image->cx, sdf_image->cy, VK_FORMAT_R8G8B8A8_UNORM, 0);

				Buffer staging_buffer(BufferTypeStaging, sdf_font_image->total_size);
				staging_buffer.map();
				memcpy(staging_buffer.mapped, sdf_image->data, staging_buffer.size);
				staging_buffer.unmap();

				release_image(sdf_image);

				VkBufferImageCopy r = {};
				r.imageExtent.width = sdf_font_image->get_cx();
				r.imageExtent.height = sdf_font_image->get_cy();
				r.imageExtent.depth = 1;
				r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				r.imageSubresource.layerCount = 1;

				auto cb = begin_once_command_buffer();
				sdf_font_image->transition_layout(cb, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				vkCmdCopyBufferToImage(cb->v, staging_buffer.v, sdf_font_image->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &r);
				sdf_font_image->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				sdf_font_image->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				end_once_command_buffer(cb);
			}
			updateDescriptorSets(&pipeline_sdf->descriptor_set->get_write(0, 0, &get_texture_info(sdf_font_image, colorSampler)));

			cmds = new CommandBuffer;

			ImGui::StyleColorsLight(nullptr);

			main_layout = new Layout;

			{
				ImGuiContext& g = *GImGui;
				ImGui::SetCurrentFont(g.IO.Fonts->Fonts[0]);
				ImGui::menubar_height = g.FontBaseSize + g.Style.FramePadding.y * 2.0f;
				ImGui::toolbar_height = 16.f + g.Style.WindowPadding.y * 2.f;
				ImGui::statusbar_height = ImGui::GetTextLineHeight() + g.Style.WindowPadding.y * 2.f;
			}

			on_resize(0, 0);
			load_layout();
			add_resize_listener(on_resize);
		}

		void end()
		{
			reset_dragging();

			show_windows();
			show_layout();

			auto _chr_count = 0;
			if (!sdf_draw_commands.empty())
			{
				for (auto &c : sdf_draw_commands)
				{
					for (auto i = 0; i < c.text.size(); i++)
					{
						auto chr = c.text[i];
						if (chr == ' ' ||
							(chr >= '0' && chr <= '9') ||
							(chr >= 'A' && chr <= 'Z') ||
							(chr >= 'a' && chr <= 'z'))
							_chr_count++;
					}
				}

				auto sdf_text_vertex_size = _chr_count * 6 * sizeof(SdfVertex);
				if (!sdf_vertex_buffer || sdf_vertex_buffer->size < sdf_text_vertex_size)
					sdf_vertex_buffer = std::make_unique<Buffer>(BufferTypeImmediateVertex, sdf_text_vertex_size);

				sdf_vertex_buffer->map(0, sdf_text_vertex_size);
				auto vtx_dst = (SdfVertex*)sdf_vertex_buffer->mapped;
				for (auto &cmd : sdf_draw_commands)
				{
					auto _chr_count = 0;
					for (auto i = 0; i < cmd.text.size(); i++)
					{
						auto chr = cmd.text[i];
						int offset;
						if (chr == ' ')
							offset = 0;
						else if (chr >= '0' && chr <= '9')
							offset = chr - '0' + 1;
						else if (chr >= 'A' && chr <= 'Z')
							offset = chr - 'A' + 1 + 10;
						else if (chr >= 'a' && chr <= 'z')
							offset = chr - 'a' + 1 + 10 + 26;
						else
							continue;
						auto w_s = glm::vec2(surface->cx, surface->cy);
						auto hs = glm::vec2(cmd.size, cmd.size) / w_s / 2.f;
						auto p = glm::vec2(cmd.x + _chr_count * cmd.size, cmd.y) / w_s;
						auto a_pos = p - hs;
						a_pos = a_pos * 2.f - 1.f;
						auto b_pos = p + glm::vec2(hs.x, -hs.y);
						b_pos = b_pos * 2.f - 1.f;
						auto c_pos = p + glm::vec2(-hs.x, hs.y);
						c_pos = c_pos * 2.f - 1.f;
						auto d_pos = p + hs;
						d_pos = d_pos * 2.f - 1.f;
						auto u0 = (float)offset / sdf_char_count;
						auto u1 = (float)(offset + 1) / sdf_char_count;
						vtx_dst[0].pos = a_pos;
						vtx_dst[0].uv = glm::vec2(u0, 0.f);
						vtx_dst[1].pos = c_pos;
						vtx_dst[1].uv = glm::vec2(u0, 1.f);
						vtx_dst[2].pos = d_pos;
						vtx_dst[2].uv = glm::vec2(u1, 1.f);
						vtx_dst[3].pos = a_pos;
						vtx_dst[3].uv = glm::vec2(u0, 0.f);
						vtx_dst[4].pos = d_pos;
						vtx_dst[4].uv = glm::vec2(u1, 1.f);
						vtx_dst[5].pos = b_pos;
						vtx_dst[5].uv = glm::vec2(u1, 0.f);
						vtx_dst += 6;
						_chr_count++;
					}
				}
				sdf_vertex_buffer->unmap();
			}

			if (main_layout->is_empty(0))
			{
				VkClearValue clear_value = {bg_color.r, bg_color.g, bg_color.b, 1.f};
				cmds->begin_renderpass(renderpass_clear.get(), framebuffers[surface->image_index].get(), &clear_value);
			}
			else
				cmds->begin_renderpass(renderpass.get(), framebuffers[surface->image_index].get());

			if (!sdf_draw_commands.empty())
			{
				cmds->set_scissor(0, 0, surface->cx, surface->cy);

				cmds->bind_vertex_buffer(sdf_vertex_buffer.get());

				cmds->bind_pipeline(pipeline_sdf);
				cmds->bind_descriptor_set();

				cmds->draw(_chr_count * 6);

				sdf_draw_commands.clear();
			}

			add_to_draw_list(cmds->v);

			accepted_mouse = ImGui::IsMouseHoveringAnyWindow();
			accepted_key = ImGui::IsAnyItemActive();
		}

		void increase_texture_ref(Texture *i)
		{
			auto index = _image_list.add(i);
			if (index == -2)
			{
				i->ui_ref_count++;
				return;
			}
			if (index == -1)
				return;

			index++;
			updateDescriptorSets(&pipeline_ui->descriptor_set->get_write(0, index, &get_texture_info(i, colorSampler)));
			i->ui_index = index;
		}

		void decrease_texture_ref(Texture *i)
		{
			if (i->ui_index != -1)
			{
				i->ui_ref_count--;
				if (i->ui_ref_count == 0)
					unregister_texture(i);
			}
		}

		void unregister_texture(Texture *i)
		{
			if (i->ui_index != -1)
			{
				updateDescriptorSets(&pipeline_ui->descriptor_set->get_write(0, i->ui_index, &get_texture_info(font_image, colorSampler)));
				_image_list.remove(i);
				i->ui_index = -1;
			}
		}

		void draw_text(const std::string &text, int x, int y, int size)
		{
			sdf_draw_commands.push_back({text, x, y, size});
		}
	}
}
