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
#include <flame/surface.h>

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <Windows.h>
#include <stdarg.h>
#include <list>

namespace flame
{
	namespace UI
	{
		void Instance::begin(int cx, int cy, float _elapsed_time)
		{
			processed_input = false;

			ImGuiIO& im_io = ImGui::GetIO();

			im_io.DisplaySize = ImVec2((float)cx, (float)cy);
			im_io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			elapsed_time = _elapsed_time;
			im_io.DeltaTime = elapsed_time;

			im_io.MousePos = ImVec2(_priv->s->mouse_x, _priv->s->mouse_y);

			im_io.MouseDown[0] = (_priv->s->mouse_buttons[0] & KeyStateDown) != 0;
			im_io.MouseDown[1] = (_priv->s->mouse_buttons[1] & KeyStateDown) != 0;
			im_io.MouseDown[2] = (_priv->s->mouse_buttons[2] & KeyStateDown) != 0;

			im_io.MouseWheel = _priv->s->mouse_scroll;

			ImGui::NewFrame();
		}

		struct Dialog
		{
			bool open;
			ShortString title;

			virtual void show() = 0;
		};

		struct MessageDialog : Dialog
		{
			ShortString message;

			virtual void show() override
			{
				ImGui::TextUnformatted(message.data);
				if (ImGui::Button("OK"))
				{
					ImGui::CloseCurrentPopup();
					open = false;
				}
			}
		};

		struct InputDialog : Dialog
		{
			ShortString label;
			MediumString input;
			std::function<void(MediumString *input)> callback;

			virtual void show() override
			{
				ImGui::InputText(label.data, input.data, sizeof(input.data));
				if (ImGui::Button("OK"))
				{
					callback(&input);
					ImGui::CloseCurrentPopup();
					open = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					open = false;
				}
			}
		};

		static std::list<std::unique_ptr<Dialog>> dialogs;

		void Instance::end()
		{
			for (auto it = dialogs.begin(); it != dialogs.end();)
			{
				auto d = (*it).get();
				if (!d->open)
				{
					ImGui::OpenPopup(d->title.data);
					d->open = true;
				}
				if (ImGui::BeginPopupModal(d->title.data, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					d->show();
					ImGui::EndPopup();
				}

				if (!d->open)
					it = dialogs.erase(it);
				else
					it++;
			}

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

		bool Instance::begin_window(const char *name, const glm::vec2 &pos, const glm::vec2 &size, bool need_save_setting)
		{
			if (!is_inf(pos.x) && !is_inf(pos.y))
				ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
			if (!is_inf(size.x) && !is_inf(size.y))
				ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
			return ImGui::Begin(name, nullptr,
				!need_save_setting ? ImGuiWindowFlags_NoSavedSettings : 0);
		}

		bool Instance::begin_plain_window(const char *name, const glm::vec2 &pos, const glm::vec2 &size)
		{
			ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
			ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			auto open = ImGui::Begin(name, nullptr,
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

		bool Instance::begin_mainmenu()
		{
			return ImGui::BeginMainMenuBar();
		}

		void Instance::end_mainmenu()
		{
			ImGui::EndMainMenuBar();
		}

		bool Instance::begin_menu(const char *label)
		{
			return ImGui::BeginMenu(label);
		}

		void Instance::end_menu()
		{
			ImGui::EndMenu();
		}

		bool Instance::menuitem(const char *label)
		{
			return ImGui::MenuItem(label);
		}

		bool Instance::button(const char *label)
		{
			return ImGui::Button(label);
		}

		bool Instance::checkbox(const char *label, bool *p)
		{
			return ImGui::Checkbox(label, p);
		}

		bool Instance::dragfloat(const char *label, float *p, float speed)
		{
			return ImGui::DragFloat(label, p, speed);
		}

		bool Instance::dragfloat2(const char *label, glm::vec2 *p, float speed)
		{
			return ImGui::DragFloat2(label, &p->x, speed);
		}

		bool Instance::dragfloat3(const char *label, glm::vec3 *p, float speed)
		{
			return ImGui::DragFloat(label, &p->x, speed);
		}

		bool Instance::dragfloat4(const char *label, glm::vec4 *p, float speed)
		{
			return ImGui::DragFloat(label, &p->x, speed);
		}

		void Instance::text_unformatted(const char *text)
		{
			ImGui::TextUnformatted(text);
		}

		void Instance::text(const char *fmt, ...)
		{
			va_list ap;
			va_start(ap, fmt);
			ImGui::TextV(fmt, ap);
			va_end(ap);
		}

		void Instance::ID_text_unformatted(const char *ID, const char *text)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				return;

			ImGuiContext& g = *GImGui;
			IM_ASSERT(text != NULL);
			const char* text_begin = text;
			auto text_end = text + strlen(text);

			const ImGuiID id = window->GetID(ID);
			const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset);
			const ImVec2 text_size = ImGui::CalcTextSize(text_begin, text_end, false, 0.f);

			ImRect bb(text_pos, text_pos + text_size);
			ImGui::ItemSize(text_size);
			if (!ImGui::ItemAdd(bb, id))
				return;

			window->DrawList->AddText(g.Font, g.FontSize, bb.Min, ImGui::GetColorU32(ImGuiCol_Text), text, text_end);
		}

		bool Instance::inputtext(const char *label, char *dst, int len)
		{
			return ImGui::InputText(label, dst, len);
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

		static glm::vec4 last_display;

		void Instance::push_displayrect(const glm::vec4 &rect)
		{
			auto &im_io = ImGui::GetIO();
			last_display.x = im_io.DisplayVisibleMin.x;
			last_display.y = im_io.DisplayVisibleMin.y;
			last_display.z = im_io.DisplayVisibleMax.x;
			last_display.w = im_io.DisplayVisibleMax.y;
			im_io.DisplayVisibleMin.x = rect.x;
			im_io.DisplayVisibleMin.y = rect.y;
			im_io.DisplayVisibleMax.x = rect.z;
			im_io.DisplayVisibleMax.y = rect.w;
		}

		void Instance::pop_displayrect()
		{
			auto &im_io = ImGui::GetIO();
			im_io.DisplayVisibleMin.x = last_display.x;
			im_io.DisplayVisibleMin.y = last_display.y;
			im_io.DisplayVisibleMax.x = last_display.z;
			im_io.DisplayVisibleMax.y = last_display.w;
		}

		void Instance::push_overlay_cliprect(const glm::vec4 &rect)
		{
			ImGui::GetOverlayDrawList()->PushClipRect(
				ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w));
		}

		void Instance::pop_overlay_cliprect()
		{
			ImGui::GetOverlayDrawList()->PopClipRect();
		}

		void Instance::add_line_to_window(const glm::vec2 &a, const glm::vec2 &b, const glm::vec4 &col)
		{
			auto wpos = ImGui::GetWindowPos();
			ImGui::GetWindowDrawList()->AddLine(ImVec2(wpos.x + a.x, wpos.y + a.y), ImVec2(wpos.x + b.x, wpos.y + b.y),
				ImColor(col.r, col.g, col.b, col.a));
		}

		static inline void add_rect_impl(ImDrawList *dl, const glm::vec4 &rect, const glm::vec4 &col)
		{
			dl->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w),
				ImColor(col.r, col.g, col.b, col.a));
		}

		void Instance::add_rect_to_window(const glm::vec4 &rect, const glm::vec4 &col)
		{
			auto wpos = ImGui::GetWindowPos();
			add_rect_impl(ImGui::GetWindowDrawList(), rect + glm::vec4(wpos.x, wpos.y, wpos.x, wpos.y), col);
		}

		void Instance::add_text_to_window(const glm::vec2 &pos, const glm::vec4 &col, const char *fmt, ...)
		{
			static char buffer[1024];

			va_list ap;
			va_start(ap, fmt);
			auto len = vsprintf(buffer, fmt, ap);
			va_end(ap);

			auto wpos = ImGui::GetWindowPos();
			ImGui::GetWindowDrawList()->AddText(ImVec2(wpos.x + pos.x, wpos.y + pos.y), ImColor(col.r, col.g, col.b, col.a),
				buffer, buffer + len);
		}

		void Instance::add_rect_to_overlap(const glm::vec4 &rect, const glm::vec4 &col)
		{
			add_rect_impl(ImGui::GetOverlayDrawList(), rect, col);
		}

		void Instance::add_message_dialog(const char *title, const char *message)
		{
			auto d = new MessageDialog;
			d->open = false;
			strcpy(d->title.data, title);
			strcpy(d->message.data, message);
			dialogs.emplace_back(d);
		}

		void Instance::add_input_dialog(const char *title, const char *label, const
			std::function<void(MediumString *input)> &callback, const char *default_input)
		{
			auto d = new InputDialog;
			d->open = false;
			strcpy(d->title.data, title);
			strcpy(d->label.data, label);
			d->callback = callback;
			if (default_input)
				strcpy(d->input.data, default_input);
			else
				d->input.data[0] = 0;
			dialogs.emplace_back(d);
		}

		Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp, Surface *s)
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

			i->_priv->s = s;

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

			s->add_keydown_listener([](Surface *, int k) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[k] = true;

				io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
				io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
				io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
				io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
			});

			s->add_keyup_listener([](Surface *, int k) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[k] = false;

				io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
				io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
				io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
				io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
			});

			s->add_char_listener([](Surface *, int c) {
				if (c == VK_TAB)
					return;

				ImGuiIO& io = ImGui::GetIO();
				if (c > 0 && c < 0x10000)
					io.AddInputCharacter((unsigned short)c);
			});

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

