#include "instance_private.h"

#include <flame/system.h>
#include <flame/math.h>
#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/queue.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <Windows.h>
#include <stdarg.h>

namespace flame
{
	namespace UI
	{
		void Instance::begin(int cx, int cy, float _elapsed_time, int mouse_x, int mouse_y,
			bool mouse_left_pressing, bool mouse_right_pressing, bool mouse_middle_pressing, int mouse_scroll)
		{
			processed_input = false;

			ImGuiIO& im_io = ImGui::GetIO();

			im_io.DisplaySize = ImVec2((float)cx, (float)cy);
			im_io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			elapsed_time = _elapsed_time;
			im_io.DeltaTime = elapsed_time;

			im_io.MousePos = ImVec2(mouse_x, mouse_y);

			im_io.MouseDown[0] = mouse_left_pressing;
			im_io.MouseDown[1] = mouse_right_pressing;
			im_io.MouseDown[2] = mouse_middle_pressing;

			im_io.MouseWheel = mouse_scroll;

			ImGui::NewFrame();
		}

		void Instance::end()
		{
			processed_input = ImGui::IsMouseHoveringAnyWindow() | ImGui::IsAnyWindowFocused();

			ImGui::Render();

			ImGuiIO& im_io = ImGui::GetIO();
			auto draw_data = ImGui::GetDrawData();

			auto vertex_size = max(draw_data->TotalVtxCount, 1) * sizeof(ImDrawVert);
			auto index_size = max(draw_data->TotalIdxCount, 1) * sizeof(ImDrawIdx);

			if (!_priv->vtx_buffer || _priv->vtx_buffer->size < vertex_size)
			{
				if (_priv->vtx_buffer)
				{
					_priv->vtx_buffer->unmap();
					graphics::destroy_buffer(_priv->d, _priv->vtx_buffer);
				}
				_priv->vtx_buffer =  graphics::create_buffer(_priv->d, vertex_size, 
					graphics::BufferUsageVertexBuffer, graphics::MemPropHost);
				_priv->vtx_buffer->map();
			}

			if (!_priv->idx_buffer || _priv->idx_buffer->size < index_size)
			{
				if (_priv->idx_buffer)
				{
					_priv->idx_buffer->unmap();
					graphics::destroy_buffer(_priv->d, _priv->idx_buffer);
				}
				_priv->idx_buffer = graphics::create_buffer(_priv->d, index_size,
					graphics::BufferUsageIndexBuffer, graphics::MemPropHost);
				_priv->idx_buffer->map();
			}

			auto vtx_dst = (ImDrawVert*)_priv->vtx_buffer->mapped;
			auto idx_dst = (ImDrawIdx*)_priv->idx_buffer->mapped;

			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const auto cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				vtx_dst += cmd_list->VtxBuffer.Size;
				idx_dst += cmd_list->IdxBuffer.Size;
			}

			_priv->vtx_buffer->flush();
			_priv->idx_buffer->flush();
		}

		void Instance::record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb)
		{
			ImGuiIO& im_io = ImGui::GetIO();
			auto draw_data = ImGui::GetDrawData();

			cb->begin_renderpass(rp, fb);
			cb->bind_pipeline(_priv->pl);
			cb->bind_descriptorset(_priv->ds);
			cb->bind_vertexbuffer(_priv->vtx_buffer);
			cb->bind_indexbuffer(_priv->idx_buffer, graphics::IndiceTypeUshort);
			cb->set_viewport(0, 0, im_io.DisplaySize.x, im_io.DisplaySize.y);
			cb->set_scissor(0, 0, im_io.DisplaySize.x, im_io.DisplaySize.y);
			glm::vec4 pc;
			pc.x = 2.f / im_io.DisplaySize.x;
			pc.y = 2.f / im_io.DisplaySize.y;
			pc.z = -1.f;
			pc.w = -1.f;
			cb->push_constant(graphics::ShaderVert, 0, sizeof(glm::vec4), &pc);

			auto vtx_offset = 0;
			auto idx_offset = 0;
			for (auto n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				for (auto cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
					if (pcmd->UserCallback)
					{
						pcmd->UserCallback(cmd_list, pcmd);
						pcmd->TextureId;
					}
					else
					{
						cb->set_scissor(
							(int)(pcmd->ClipRect.x),
							(int)(pcmd->ClipRect.y),
							(uint)(pcmd->ClipRect.z - pcmd->ClipRect.x),
							(uint)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1)  // TODO: + 1??????
						);
						cb->draw_indexed(pcmd->ElemCount, idx_offset, vtx_offset, 1, (int)pcmd->TextureId);
					}
					idx_offset += pcmd->ElemCount;
				}
				vtx_offset += cmd_list->VtxBuffer.Size;
			}

			cb->end_renderpass();
		}

		bool Instance::begin_window(const char *title, const glm::vec2 &pos, const glm::vec2 &size, bool need_save_setting)
		{
			if (!is_inf(pos.x) && !is_inf(pos.y))
				ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
			if (!is_inf(size.x) && !is_inf(size.y))
				ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
			return ImGui::Begin(title, nullptr,
				!need_save_setting ? ImGuiWindowFlags_NoSavedSettings : 0);
		}

		bool Instance::begin_plain_window(const char *title, const glm::vec2 &pos, const glm::vec2 &size)
		{
			ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
			ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			auto open = ImGui::Begin(title, nullptr, 
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse |
				ImGuiWindowFlags_NoTitleBar);
			ImGui::PopStyleVar();
			return open;
		}

		void Instance::end_window()
		{
			ImGui::End();
		}

		bool Instance::button(const char *title)
		{
			return ImGui::Button(title);
		}

		bool Instance::checkbox(const char *title, bool *p)
		{
			return ImGui::Checkbox(title, p);
		}

		bool Instance::dragfloat(const char *title, float *p, float speed)
		{
			return ImGui::DragFloat(title, p, speed);
		}

		bool Instance::dragfloat2(const char *title, glm::vec2 *p, float speed)
		{
			return ImGui::DragFloat2(title, &p->x, speed);
		}

		bool Instance::dragfloat3(const char *title, glm::vec3 *p, float speed)
		{
			return ImGui::DragFloat(title, &p->x, speed);
		}

		bool Instance::dragfloat4(const char *title, glm::vec4 *p, float speed)
		{
			return ImGui::DragFloat(title, &p->x, speed);
		}

		void Instance::text(const char *fmt, ...)
		{
			va_list ap;
			va_start(ap, fmt);
			ImGui::TextV(fmt, ap);
			va_end(ap);
		}

		bool Instance::inputtext(const char *title, char *dst, int len)
		{
			return ImGui::InputText(title, dst, len);
		}

		unsigned int Instance::get_last_ID()
		{
			return GImGui->CurrentWindow->DC.LastItemId;
		}

		bool Instance::is_last_item_focused()
		{
			return ImGui::IsItemFocused();
		}

		bool Instance::is_curr_window_focused()
		{
			return ImGui::IsWindowFocused();
		}

		bool Instance::is_last_item_hovered()
		{
			return ImGui::IsItemHovered();
		}

		bool Instance::is_curr_window_hovered()
		{
			return ImGui::IsWindowHovered();
		}

		glm::vec4 Instance::get_last_item_rect()
		{
			auto LT = ImGui::GetItemRectMin();
			auto RB = ImGui::GetItemRectMax();
			return glm::vec4(LT.x, LT.y, RB.x, RB.y);
		}

		glm::vec4 Instance::get_curr_window_rect()
		{
			auto LT = ImGui::GetWindowPos();
			auto RB = ImGui::GetWindowSize();
			return glm::vec4(LT.x, LT.y, LT.x + RB.x, LT.y + RB.y);
		}

		void Instance::add_line_to_window(const glm::vec2 &a, const glm::vec2 &b, const glm::vec4 &col)
		{
			ImGui::GetWindowDrawList()->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y),
				ImColor(col.r, col.g, col.b, col.a));
		}

		static inline void add_rect_impl(ImDrawList *dl, const glm::vec4 &rect, const glm::vec4 &col)
		{
			dl->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w),
				ImColor(col.r, col.g, col.b, col.a));
		}

		void Instance::add_rect_to_window(const glm::vec4 &rect, const glm::vec4 &col)
		{
			add_rect_impl(ImGui::GetWindowDrawList(), rect, col);
		}

		void Instance::add_text_to_window(const glm::vec2 &pos, const glm::vec4 &col, const char *fmt, ...)
		{
			static char buffer[1024];

			va_list ap;
			va_start(ap, fmt);
			auto len = vsprintf(buffer, fmt, ap);
			va_end(ap);

			ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x, pos.y), ImColor(col.r, col.g, col.b, col.a),
				buffer, buffer + len);
		}

		void Instance::add_rect_to_overlap(const glm::vec4 &rect, const glm::vec4 &col)
		{
			add_rect_impl(ImGui::GetOverlayDrawList(), rect, col);
		}

		Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp)
		{
			auto i = new Instance;

			i->_priv = new InstancePrivate;
			i->_priv->d = d;

			i->_priv->vert = graphics::create_shader(d, "ui.vert");
			i->_priv->vert->build();
			i->_priv->frag = graphics::create_shader(d, "ui.frag");
			i->_priv->frag->build();

			i->_priv->pl = create_pipeline(d);
			i->_priv->pl->set_vertex_attributes({{
				graphics::VertexAttributeFloat2,
				graphics::VertexAttributeFloat2,
				graphics::VertexAttributeByte4
			}});
			i->_priv->pl->set_size(0, 0);
			i->_priv->pl->set_cull_mode(graphics::CullModeNone);
			i->_priv->pl->set_blend_state(0, true, graphics::BlendFactorSrcAlpha, graphics::BlendFactorOneMinusSrcAlpha,
				graphics::BlendFactorZero, graphics::BlendFactorOneMinusSrcAlpha);
			i->_priv->pl->set_dynamic_state({graphics::DynamicStateScissor});
			i->_priv->pl->add_shader(i->_priv->vert);
			i->_priv->pl->add_shader(i->_priv->frag);
			i->_priv->pl->set_renderpass(rp, 0);
			i->_priv->pl->build_graphics();

			i->_priv->ds = d->dp->create_descriptorset(i->_priv->pl, 0);

			ImGui::CreateContext();
			auto &im_io = ImGui::GetIO();
			im_io.Fonts->AddFontDefault();
			unsigned char* font_pixels; int font_tex_width, font_tex_height;
			im_io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_tex_width, &font_tex_height);
			i->_priv->font_tex = graphics::create_texture(d, font_tex_width, font_tex_height, 
				1, 1, graphics::Format_R8G8B8A8_UNORM, graphics::TextureUsageShaderSampled |
				graphics::TextureUsageTransferDst, graphics::MemPropDevice);

			auto font_stag = graphics::create_buffer(d, font_tex_width * font_tex_height * 4, 
				graphics::BufferUsageTransferSrc, graphics::MemPropHost | graphics::MemPropHostCoherent);
			font_stag->map();
			memcpy(font_stag->mapped, font_pixels, font_stag->size);
			font_stag->unmap();

			auto cb_cpy = d->cp->create_commandbuffer();
			cb_cpy->begin(true);
			cb_cpy->change_texture_layout(i->_priv->font_tex, graphics::TextureLayoutUndefined, graphics::TextureLayoutTransferDst);
			graphics::BufferImageCopy font_cpy_range;
			font_cpy_range.buffer_offset = 0;
			font_cpy_range.image_width = font_tex_width;
			font_cpy_range.image_height = font_tex_height;
			font_cpy_range.image_level = 0;
			cb_cpy->copy_buffer_to_image(font_stag, i->_priv->font_tex, 1, &font_cpy_range);
			cb_cpy->change_texture_layout(i->_priv->font_tex, graphics::TextureLayoutTransferDst, graphics::TextureLayoutShaderReadOnly);
			cb_cpy->end();
			d->q->submit(cb_cpy, nullptr, nullptr);
			d->q->wait_idle();
			d->cp->destroy_commandbuffer(cb_cpy);

			graphics::destroy_buffer(d, font_stag);

			im_io.Fonts->TexID = (void*)0; // image index

			i->_priv->font_view = graphics::create_textureview(d, i->_priv->font_tex);

			i->_priv->font_sam = graphics::create_sampler(d, graphics::FilterLinear, graphics::FilterLinear, false);

			for (auto j = 0; j < 128; j++)
				i->_priv->ds->set_texture(0, j, i->_priv->font_view, i->_priv->font_sam);

			i->_priv->vtx_buffer = nullptr;
			i->_priv->idx_buffer = nullptr;

			im_io.KeyMap[ImGuiKey_Tab] = VK_TAB;
			im_io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			im_io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			im_io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			im_io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			im_io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			im_io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			im_io.KeyMap[ImGuiKey_Home] = VK_HOME;
			im_io.KeyMap[ImGuiKey_End] = VK_END;
			im_io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			im_io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			im_io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			im_io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			im_io.KeyMap[ImGuiKey_A] = 'A';
			im_io.KeyMap[ImGuiKey_C] = 'C';
			im_io.KeyMap[ImGuiKey_V] = 'V';
			im_io.KeyMap[ImGuiKey_X] = 'X';
			im_io.KeyMap[ImGuiKey_Y] = 'Y';
			im_io.KeyMap[ImGuiKey_Z] = 'Z';
			im_io.SetClipboardTextFn = [](void *user_data, const char *s) {
				set_clipboard(s);
			};
			im_io.GetClipboardTextFn = [](void *user_data) {
				static LongString s;
				get_clipboard(&s);
				return (const char*)s.data;
			};

			return i;
		}

		void destroy_instance(graphics::Device *d, Instance *i)
		{
			if (i->_priv->vtx_buffer)
				graphics::destroy_buffer(d, i->_priv->vtx_buffer);
			if (i->_priv->idx_buffer)
				graphics::destroy_buffer(d, i->_priv->idx_buffer);
			graphics::destroy_sampler(d, i->_priv->font_sam);
			graphics::destroy_textureview(d, i->_priv->font_view);
			graphics::destroy_texture(d, i->_priv->font_tex);
			d->dp->destroy_descriptorset(i->_priv->ds);
			graphics::destroy_shader(d, i->_priv->vert);
			graphics::destroy_shader(d, i->_priv->frag);
			graphics::destroy_pipeline(d, i->_priv->pl);

			delete i->_priv;
			delete i;
		}
	}
}

