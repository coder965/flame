//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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

#include <Windows.h>
#include <stdarg.h>
#include <list>

namespace flame
{
	namespace UI
	{
		void Drawlist::push_cliprect(const Rect &rect)
		{
			auto dl = (ImDrawList*)_priv;
			dl->PushClipRect(
				ImVec2(rect.min.x, rect.min.y), ImVec2(rect.max.x, rect.max.y));
		}

		void Drawlist::pop_cliprect()
		{
			auto dl = (ImDrawList*)_priv;
			dl->PopClipRect();
		}

		void Drawlist::add_line(const Vec2 &a, const Vec2 &b, const Vec4 &col)
		{
			auto dl = (ImDrawList*)_priv;
			dl->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y),
				ImColor(col.x, col.y, col.z, col.w));
		}

		void Drawlist::add_rect(const Rect &rect, const Vec4 &col, float round, bool round_LT, bool round_RT, bool round_LB, bool round_RB)
		{
			auto corner_flags = 0;
			if (round_LT)
				corner_flags |= ImDrawCornerFlags_TopLeft;
			if (round_RT)
				corner_flags |= ImDrawCornerFlags_TopRight;
			if (round_LB)
				corner_flags |= ImDrawCornerFlags_BotLeft;
			if (round_RB)
				corner_flags |= ImDrawCornerFlags_BotRight;
			auto dl = (ImDrawList*)_priv;
			dl->AddRect(ImVec2(rect.min.x, rect.min.y), ImVec2(rect.max.x, rect.max.y),
				ImColor(col.x, col.y, col.z, col.w), round, corner_flags);
		}

		void Drawlist::add_rect_filled(const Rect &rect, const Vec4 &col, float round, bool round_LT, bool round_RT, bool round_LB, bool round_RB)
		{
			auto corner_flags = 0;
			if (round_LT)
				corner_flags |= ImDrawCornerFlags_TopLeft;
			if (round_RT)
				corner_flags |= ImDrawCornerFlags_TopRight;
			if (round_LB)
				corner_flags |= ImDrawCornerFlags_BotLeft;
			if (round_RB)
				corner_flags |= ImDrawCornerFlags_BotRight;
			auto dl = (ImDrawList*)_priv;
			dl->AddRectFilled(ImVec2(rect.min.x, rect.min.y), ImVec2(rect.max.x, rect.max.y),
				ImColor(col.x, col.y, col.z, col.w), round, corner_flags);
		}

		void Drawlist::add_circle(const Vec2 &center, float radius, const Vec4 &col)
		{
			auto dl = (ImDrawList*)_priv;
			dl->AddCircle(ImVec2(center.x, center.y), radius,
				ImColor(col.x, col.y, col.z, col.w));
		}

		void Drawlist::add_circle_filled(const Vec2 &center, float radius, const Vec4 &col)
		{
			auto dl = (ImDrawList*)_priv;
			dl->AddCircleFilled(ImVec2(center.x, center.y), radius,
				ImColor(col.x, col.y, col.z, col.w));
		}

		void Drawlist::add_bezier(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec4 &col, float thickness)
		{
			auto dl = (ImDrawList*)_priv;
			dl->AddBezierCurve(ImVec2(p0.x, p0.y), ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), ImVec2(p3.x, p3.y),
				ImColor(col.x, col.y, col.z, col.w), thickness);
		}

		void Drawlist::add_text(const Vec2 &pos, const Vec4 &col, const char *fmt, ...)
		{
			static char buffer[1024];

			va_list ap;
			va_start(ap, fmt);
			auto len = vsprintf(buffer, fmt, ap);
			va_end(ap);

			auto dl = (ImDrawList*)_priv;
			dl->AddText(ImVec2(pos.x, pos.y),
				ImColor(col.x, col.y, col.z, col.w), buffer, buffer + len);
		}

		void Drawlist::add_image()
		{
			auto dl = (ImDrawList*)_priv;
			dl->AddImage(ImTextureID(1), ImVec2(0, 0), ImVec2(100, 100));
		}

		void Drawlist::draw_grid(const Vec2 &wnd_off, const Vec2 &off, const Vec2 &size)
		{
			for (auto i = mod((int)off.x, 100); i.y < size.x; i.y += 100, i.x--)
			{
				if (i.y < 0)
					continue;
				add_line(Vec2(i.y, 0.f) + wnd_off, Vec2(i.y, size.y) + wnd_off, Vec4(1.f));
				add_text(Vec2(i.y + 4, 0.f) + wnd_off, Vec4(1.f), "%d", i.x * -100);
			}
			for (auto i = mod((int)off.y, 100); i.y < size.y; i.y += 100, i.x--)
			{
				if (i.y < 0)
					continue;
				add_line(Vec2(0.f, i.y) + wnd_off, Vec2(size.x, i.y) + wnd_off, Vec4(1.f));
				add_text(Vec2(4.f, i.y) + wnd_off, Vec4(1.f), "%d", i.x * -100);
			}
		}

		void Instance::add_font(const char *ttf_filename, int code_min, int code_max)
		{
			auto &im_io = ImGui::GetIO();
			static const ImWchar ranges[] = {
				code_min,
				code_max,
				0
			};
			ImFontConfig config;
			config.MergeMode = true;
			config.PixelSnapH = true;
			im_io.Fonts->AddFontFromFileTTF(ttf_filename, 16.f, &config, ranges);
		}

		void Instance::build()
		{
			auto &im_io = ImGui::GetIO();

			unsigned char* font_pixels; int font_tex_width, font_tex_height;
			im_io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_tex_width, &font_tex_height);
			_priv->font_tex = graphics::create_texture(_priv->d, Ivec2(font_tex_width, font_tex_height),
				1, 1, graphics::Format_R8G8B8A8_UNORM, graphics::TextureUsageShaderSampled |
				graphics::TextureUsageTransferDst, graphics::MemPropDevice);

#if defined(FLAME_GRAPHICS_VULKAN)
			auto font_stag = graphics::create_buffer(_priv->d, font_tex_width * font_tex_height * 4,
				graphics::BufferUsageTransferSrc, graphics::MemPropHost | graphics::MemPropHostCoherent);
			font_stag->map();
			memcpy(font_stag->mapped, font_pixels, font_stag->size);
			font_stag->unmap();

			auto cb_cpy = _priv->d->cp->create_commandbuffer();
			cb_cpy->begin(true);
			cb_cpy->change_texture_layout(_priv->font_tex, graphics::TextureLayoutUndefined, graphics::TextureLayoutTransferDst);
			graphics::BufferImageCopy font_cpy_range;
			font_cpy_range.buffer_offset = 0;
			font_cpy_range.image_width = font_tex_width;
			font_cpy_range.image_height = font_tex_height;
			font_cpy_range.image_level = 0;
			cb_cpy->copy_buffer_to_image(font_stag, _priv->font_tex, 1, &font_cpy_range);
			cb_cpy->change_texture_layout(_priv->font_tex, graphics::TextureLayoutTransferDst, graphics::TextureLayoutShaderReadOnly);
			cb_cpy->end();
			_priv->d->q->submit(cb_cpy, nullptr, nullptr);
			_priv->d->q->wait_idle();
			_priv->d->cp->destroy_commandbuffer(cb_cpy);

			graphics::destroy_buffer(_priv->d, font_stag);

			im_io.Fonts->TexID = (void*)0; // image index

			_priv->font_view = graphics::create_textureview(_priv->d, _priv->font_tex);

			_priv->font_sam = graphics::create_sampler(_priv->d, graphics::FilterLinear, graphics::FilterLinear, false);
#else
			_priv->font_tex->set_data(font_pixels);
#endif

#if defined(FLAME_GRAPHICS_VULKAN)
			for (auto j = 0; j < 128; j++)
				_priv->ds->set_texture(0, j, _priv->font_view, _priv->font_sam);
#endif
		}

		void Instance::set_texture(int index, graphics::Textureview *tv)
		{
#if defined(FLAME_GRAPHICS_VULKAN)
			_priv->ds->set_texture(0, index, tv ? tv : _priv->font_view, _priv->font_sam);
#endif
		}

		void Instance::begin(int cx, int cy, float _elapsed_time)
		{
			processed_mouse_input = false;
			processed_keyboard_input = false;

			auto &im_io = ImGui::GetIO();

			im_io.DisplaySize = ImVec2((float)cx, (float)cy);
			im_io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			elapsed_time = _elapsed_time;
			im_io.DeltaTime = elapsed_time;

			im_io.MousePos = ImVec2(_priv->s->mouse_pos.x, _priv->s->mouse_pos.y);

			im_io.MouseDown[0] = (_priv->s->mouse_buttons[0] & KeyStateDown) != 0;
			im_io.MouseDown[1] = (_priv->s->mouse_buttons[1] & KeyStateDown) != 0;
			im_io.MouseDown[2] = (_priv->s->mouse_buttons[2] & KeyStateDown) != 0;

			im_io.MouseWheel = _priv->s->mouse_scroll;

			if ((im_io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
			{
				static ImGuiMouseCursor last_cursor = ImGuiMouseCursor_Arrow;
				auto cursor = ImGui::GetMouseCursor();
				if (last_cursor != cursor)
				{
					if (im_io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
						_priv->s->show_cursor(false);
					else
					{
						_priv->s->set_cursor(_priv->cursors[cursor]);
						_priv->s->show_cursor(true);
					}
					last_cursor = cursor;
				}
			}

			ImGui::NewFrame();
		}

		struct Dialog
		{
			bool open;
			ShortString title;
			void *user_data;
			std::function<void(Instance *ui, void *user_data, bool &open)> show_callback;

			Dialog(const char *_title, int user_data_size,
				const std::function<void(Instance *ui, void *user_data, bool &open)> &_show_callback) :
				open(false),
				show_callback(_show_callback)
			{
				strcpy(title.data, _title);
				user_data = malloc(user_data_size);
			}

			~Dialog()
			{
				free(user_data);
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
					d->show_callback(this, d->user_data, d->open);
					if (!d->open)
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}

				if (!d->open)
					it = dialogs.erase(it);
				else
					it++;
			}

			processed_mouse_input = ImGui::IsMouseHoveringAnyWindow() | ImGui::IsAnyWindowFocused();
			processed_keyboard_input = ImGui::IsAnyItemActive();

			ImGui::Render();

#if defined(FLAME_GRAPHICS_VULKAN)
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
				_priv->vtx_buffer = graphics::create_buffer(_priv->d, vertex_size,
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
#endif
		}

#if defined(FLAME_GRAPHICS_VULKAN)
		void Instance::record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb)
		{
			ImGuiIO& im_io = ImGui::GetIO();
			auto draw_data = ImGui::GetDrawData();

			cb->begin_renderpass(rp, fb);
			cb->bind_pipeline(_priv->pl);
			cb->bind_descriptorset(_priv->ds);
			cb->bind_vertexbuffer(_priv->vtx_buffer);
			cb->bind_indexbuffer(_priv->idx_buffer, graphics::IndiceTypeUshort);
			cb->set_viewport(Ivec2(0), Ivec2(im_io.DisplaySize.x, im_io.DisplaySize.y));
			cb->set_scissor(Ivec2(0), Ivec2(im_io.DisplaySize.x, im_io.DisplaySize.y));
			Vec4 pc;
			pc.x = 2.f / im_io.DisplaySize.x;
			pc.y = 2.f / im_io.DisplaySize.y;
			pc.z = -1.f;
			pc.w = -1.f;
			cb->push_constant(graphics::ShaderVert, 0, sizeof(Vec4), &pc);

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
						cb->set_scissor(Ivec2((int)(pcmd->ClipRect.x), (int)(pcmd->ClipRect.y)),
							Ivec2((uint)(pcmd->ClipRect.z - pcmd->ClipRect.x),
							(uint)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1))  // TODO: + 1??????
						);
						cb->draw_indexed(pcmd->ElemCount, idx_offset, vtx_offset, 1, (int)pcmd->TextureId);
					}
					idx_offset += pcmd->ElemCount;
				}
				vtx_offset += cmd_list->VtxBuffer.Size;
			}

			cb->end_renderpass();
		}
#else
		void Instance::render()
		{

		}
#endif

		bool Instance::begin_window(const char *name, const Vec2 &pos, const Vec2 &size, int flags)
		{
			if (!is_inf(pos.x) && !is_inf(pos.y))
				ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
			if (!is_inf(size.x) && !is_inf(size.y))
				ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
			return ImGui::Begin(name, nullptr,
				((flags & WindowSaveSetting) == 0 ? ImGuiWindowFlags_NoSavedSettings : 0) |
				((flags & WindowNoResize) != 0 ? ImGuiWindowFlags_NoResize : 0) |
				((flags & WindowNoMove) != 0 ? ImGuiWindowFlags_NoMove : 0));
		}

		bool Instance::begin_plain_window(const char *name, const Vec2 &pos, const Vec2 &size)
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

		bool Instance::begin_status_window()
		{
			ImGuiIO& im_io = ImGui::GetIO();
			ImGuiContext& im_g = *GImGui;
			auto height = im_g.FontSize + im_g.Style.WindowPadding.y * 2.f;
			ImGui::SetNextWindowPos(ImVec2(0.f, im_io.DisplaySize.y - height));
			ImGui::SetNextWindowSize(ImVec2(im_io.DisplaySize.x, height));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			auto open = ImGui::Begin("##status", nullptr,
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

		bool Instance::begin_sidebarL_window(int base_y)
		{
			ImGuiIO& im_io = ImGui::GetIO();
			ImGuiContext& im_g = *GImGui;
			auto width = im_g.FontSize + im_g.Style.WindowPadding.y * 2.f;
			ImGui::SetNextWindowPos(ImVec2(0.f, base_y));
			ImGui::SetNextWindowSize(ImVec2(width, im_io.DisplaySize.y - base_y));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			auto open = ImGui::Begin("##sidebarL", nullptr,
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

		bool Instance::menuitem(const char *label, const char *shortcut, bool checked)
		{
			return ImGui::MenuItem(label, shortcut, checked);
		}

		void Instance::begin_tabbar(const char *str_id)
		{
			ImGui::BeginTabBar(str_id);
		}

		void Instance::end_tabbar()
		{
			ImGui::EndTabBar();
		}

		bool Instance::tabitem(const char *label)
		{
			return ImGui::TabItem(label);
		}

		bool Instance::begin_child(const char *str_id, const Vec2 &size, bool border)
		{
			return ImGui::BeginChild(str_id, ImVec2(size.x, size.y), border);
		}

		void Instance::end_child()
		{
			ImGui::EndChild();
		}

		void Instance::separator()
		{
			ImGui::Separator();
		}

		bool Instance::button(const char *label)
		{
			return ImGui::Button(label);
		}

		bool Instance::checkbox(const char *label, bool *p)
		{
			return ImGui::Checkbox(label, p);
		}

		bool Instance::dragfloat(const char *label, float *p, float speed, float v_min, float v_max)
		{
			return ImGui::DragFloat(label, p, speed, v_min, v_max);
		}

		bool Instance::dragfloat2(const char *label, Vec2 *p, float speed, float v_min, float v_max)
		{
			return ImGui::DragFloat2(label, &p->x, speed, v_min, v_max);
		}

		bool Instance::dragfloat3(const char *label, Vec3 *p, float speed, float v_min, float v_max)
		{
			return ImGui::DragFloat(label, &p->x, speed, v_min, v_max);
		}

		bool Instance::dragfloat4(const char *label, Vec4 *p, float speed, float v_min, float v_max)
		{
			return ImGui::DragFloat(label, &p->x, speed, v_min, v_max);
		}

		bool Instance::dragint(const char *label, int *p, float speed, int v_min, int v_max)
		{
			return ImGui::DragInt(label, p, speed, v_min, v_max);
		}

		bool Instance::dragint2(const char *label, Ivec2 *p, float speed, int v_min, int v_max)
		{
			return ImGui::DragInt2(label, &p->x, speed, v_min, v_max);
		}

		bool Instance::dragint3(const char *label, Ivec3 *p, float speed, int v_min, int v_max)
		{
			return ImGui::DragInt3(label, &p->x, speed, v_min, v_max);
		}

		bool Instance::dragint4(const char *label, Ivec4 *p, float speed, int v_min, int v_max)
		{
			return ImGui::DragInt4(label, &p->x, speed, v_min, v_max);
		}

		void Instance::text_unformatted(const char *text)
		{
			ImGui::TextUnformatted(text);
		}

		void Instance::text_unformatted_RA(const char *text)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				return;

			ImGuiContext& g = *GImGui;

			auto text_begin = text;
			auto text_end = text + strlen(text);

			const ImVec2 text_size = ImGui::CalcTextSize(text_begin, text_end, false, 0.f);
			auto pos = ImGui::GetCursorScreenPos();
			pos.x -= text_size.x;
			ImGui::SetCursorScreenPos(pos);

			const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset);

			ImRect bb(text_pos, text_pos + text_size);
			ImGui::ItemSize(text_size);
			if (!ImGui::ItemAdd(bb, 0))
				return;

			ImGui::RenderTextWrapped(bb.Min, text_begin, text_end, 0.f);
		}

		void Instance::text(const char *fmt, ...)
		{
			va_list ap;
			va_start(ap, fmt);
			ImGui::TextV(fmt, ap);
			va_end(ap);
		}

		void Instance::ID_text_unformatted(const char *str_id, const char *text)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				return;

			ImGuiContext& g = *GImGui;
			IM_ASSERT(text != NULL);
			const char* text_begin = text;
			auto text_end = text + strlen(text);

			const ImGuiID id = window->GetID(str_id);
			const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset);
			const ImVec2 text_size = ImGui::CalcTextSize(text_begin, text_end, false, 0.f);

			ImRect bb(text_pos, text_pos + text_size);
			ImGui::ItemSize(text_size);
			if (!ImGui::ItemAdd(bb, id))
				return;

			window->DrawList->AddText(g.Font, g.FontSize, bb.Min, ImGui::GetColorU32(ImGuiCol_Text), text, text_end);
		}

		bool Instance::inputtext(const char *label, char *dst, int len, bool auto_select_all)
		{
			return ImGui::InputText(label, dst, len, auto_select_all ? ImGuiInputTextFlags_AutoSelectAll : 0);
		}

		bool Instance::selectable(const char *label, bool selected)
		{
			return ImGui::Selectable(label, selected);
		}

		void Instance::image(int index, const Vec2 &size)
		{
			ImGui::Image(ImTextureID(index), ImVec2(size.x, size.y));
		}

		void Instance::invisibleitem(const char *str_id, const Vec2 &size)
		{
			ImGui::InvisibleButton(str_id, ImVec2(size.x, size.y));
		}

		Vec2 Instance::get_cursor_pos()
		{
			auto pos = ImGui::GetCursorScreenPos();
			return Vec2(pos.x, pos.y);
		}

		void Instance::set_cursor_pos(const Vec2 &pos)
		{
			ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
		}

		void Instance::sameline()
		{
			ImGui::SameLine();
		}

		void Instance::push_item_width(float width)
		{
			ImGui::PushItemWidth(width);
		}

		void Instance::pop_item_width()
		{
			ImGui::PopItemWidth();
		}

		void Instance::push_ID(int ID)
		{
			ImGui::PushID(ID);
		}

		void Instance::pop_ID()
		{
			ImGui::PopID();
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

		bool Instance::is_last_item_active()
		{
			return ImGui::IsItemActive();
		}

		Rect Instance::get_last_item_rect()
		{
			auto LT = ImGui::GetItemRectMin();
			auto RB = ImGui::GetItemRectMax();
			return Rect(LT.x, LT.y, RB.x, RB.y);
		}

		Rect Instance::get_curr_window_rect()
		{
			auto LT = ImGui::GetWindowPos();
			auto RB = ImGui::GetWindowSize();
			return Rect(LT.x, LT.y, LT.x + RB.x, LT.y + RB.y);
		}

		Rect Instance::get_curr_window_inner_rect()
		{
			auto pos = ImGui::GetWindowPos();
			auto LT = ImGui::GetWindowContentRegionMin() + pos;
			auto RB = ImGui::GetWindowContentRegionMax() + pos;
			return Rect(LT.x, LT.y, RB.x, RB.y);
		}

		static Vec4 last_display;

		Rect Instance::set_global_cliprect(const Rect &rect)
		{
			auto &im_io = ImGui::GetIO();
			Rect last(im_io.DisplayVisibleMin.x,
				im_io.DisplayVisibleMin.y,
				im_io.DisplayVisibleMax.x,
				im_io.DisplayVisibleMax.y);
			im_io.DisplayVisibleMin.x = rect.min.x;
			im_io.DisplayVisibleMin.y = rect.min.y;
			im_io.DisplayVisibleMax.x = rect.max.x;
			im_io.DisplayVisibleMax.y = rect.max.y;
			return last;
		}

		Drawlist Instance::get_overlap_drawlist()
		{
			Drawlist dl;
			dl._priv = (DrawlistPrivate*)ImGui::GetOverlayDrawList();
			return dl;
		}

		Drawlist Instance::get_curr_window_drawlist()
		{
			Drawlist dl;
			dl._priv = (DrawlistPrivate*)ImGui::GetWindowDrawList();
			return dl;
		}

		void *Instance::add_dialog(const char *title, int user_data_size,
			const std::function<void(Instance *ui, void *user_data, bool &open)> &show_callback)
		{
			auto d = new Dialog(title, user_data_size, show_callback);
			dialogs.emplace_back(d);
			return d->user_data;
		}

		struct MessageDialogData
		{
			MediumString message;

			MessageDialogData()
			{
			}

			~MessageDialogData()
			{
			}
		};

		void Instance::add_message_dialog(const char *title, const char *message)
		{
			auto user_data = add_dialog(title, sizeof(MessageDialogData), [](Instance *ui, void *user_data, bool &open) {
				auto dialog_data = (MessageDialogData*)user_data;
				ui->text_unformatted(dialog_data->message.data);
				if (ui->button("OK"))
				{
					dialog_data->~MessageDialogData(); // pod struct do not need to use destruct, here just for demonstration
					open = false;
				}
			});
			auto dialog_data = (MessageDialogData*)user_data;
			new (dialog_data) MessageDialogData(); // pod struct do not need to use new, here just for demonstration
			strcpy(dialog_data->message.data, message);
		}

		struct InputDialogData
		{
			ShortString label;
			MediumString input;
			std::function<void(MediumString *input)> callback;
		};

		void Instance::add_input_dialog(const char *title, const char *label, const
			std::function<void(MediumString *input)> &callback, const char *default_input)
		{
			auto user_data = add_dialog(title, sizeof(InputDialogData), [](Instance *ui, void *user_data, bool &open) {
				auto dialog_data = (InputDialogData*)user_data;
				ui->inputtext(dialog_data->label.data, dialog_data->input.data,
					sizeof(dialog_data->input.data), true);
				if (ui->button("OK"))
				{
					dialog_data->callback(&dialog_data->input);
					open = false;
				}
				ui->sameline();
				if (ui->button("Cancel"))
					open = false;

				if (!open)
					dialog_data->~InputDialogData();
			});
			auto dialog_data = (InputDialogData*)user_data;
			new (dialog_data) InputDialogData();
			strcpy(dialog_data->label.data, label);
			dialog_data->callback = callback;
			if (default_input)
				strcpy(dialog_data->input.data, default_input);
			else
				dialog_data->input.data[0] = 0;

		}

		void Instance::set_mousecursor(CursorType type)
		{
			ImGuiMouseCursor c;
			switch (type)
			{
			case CursorNone:
				c = ImGuiMouseCursor_None;
				break;
			case CursorArrow:
				c = ImGuiMouseCursor_Arrow;
				break;
			case CursorIBeam:
				c = ImGuiMouseCursor_TextInput;
				break;
			case CursorSizeAll:
				c = ImGuiMouseCursor_ResizeAll;
				break;
			case CursorSizeNS:
				c = ImGuiMouseCursor_ResizeNS;
				break;
			case CursorSizeWE:
				c = ImGuiMouseCursor_ResizeEW;
				break;
			case CursorSizeNESW:
				c = ImGuiMouseCursor_ResizeNESW;
				break;
			case CursorSizeNWSE:
				c = ImGuiMouseCursor_ResizeNWSE;
				break;
			default:
				c = ImGuiMouseCursor_Arrow;
			}
			ImGui::SetMouseCursor(c);
		}

		Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp, Surface *s)
		{
			auto i = new Instance;

			i->_priv = new InstancePrivate;
			i->_priv->d = d;

			i->_priv->vert = graphics::create_shader(d, "ui.glsl3.vert");
			i->_priv->vert->build();
			i->_priv->frag = graphics::create_shader(d, "ui.glsl3.frag");
			i->_priv->frag->build();

			i->_priv->pl = create_pipeline(d);
			i->_priv->pl->set_vertex_attributes({ {
				graphics::VertexAttributeFloat2,
				graphics::VertexAttributeFloat2,
				graphics::VertexAttributeByte4
			} });
			i->_priv->pl->set_size(Ivec2(0));
			i->_priv->pl->set_cull_mode(graphics::CullModeNone);
			i->_priv->pl->set_blend_state(0, true, graphics::BlendFactorSrcAlpha, graphics::BlendFactorOneMinusSrcAlpha,
				graphics::BlendFactorZero, graphics::BlendFactorOneMinusSrcAlpha);
			i->_priv->pl->set_dynamic_state({ graphics::DynamicStateScissor });
			i->_priv->pl->add_shader(i->_priv->vert);
			i->_priv->pl->add_shader(i->_priv->frag);
			i->_priv->pl->set_renderpass(rp, 0);
			i->_priv->pl->build_graphics();

#if defined(FLAME_GRAPHICS_VULKAN)
			i->_priv->ds = d->dp->create_descriptorset(i->_priv->pl, 0);
#endif

			ImGui::CreateContext();
			auto &im_io = ImGui::GetIO();
			im_io.Fonts->AddFontDefault();

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

			im_io.SetClipboardTextFn = [](void *user_data, const char *s) {
				set_clipboard(s);
			};
			im_io.GetClipboardTextFn = [](void *user_data) {
				static LongString s;
				get_clipboard(&s);
				return (const char*)s.data;
			};

			im_io.ImeWindowHandle = s->get_win32_handle();

			i->_priv->cursors[ImGuiMouseCursor_Arrow] = s->get_standard_cursor(CursorArrow);
			i->_priv->cursors[ImGuiMouseCursor_TextInput] = s->get_standard_cursor(CursorIBeam);
			i->_priv->cursors[ImGuiMouseCursor_ResizeAll] = s->get_standard_cursor(CursorSizeAll);
			i->_priv->cursors[ImGuiMouseCursor_ResizeNS] = s->get_standard_cursor(CursorSizeNS);
			i->_priv->cursors[ImGuiMouseCursor_ResizeEW] = s->get_standard_cursor(CursorSizeWE);
			i->_priv->cursors[ImGuiMouseCursor_ResizeNESW] = s->get_standard_cursor(CursorSizeNESW);
			i->_priv->cursors[ImGuiMouseCursor_ResizeNWSE] = s->get_standard_cursor(CursorSizeNWSE);

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

			return i;
		}

		void destroy_instance(graphics::Device *d, Instance *i)
		{
			if (i->_priv->vtx_buffer)
				graphics::destroy_buffer(d, i->_priv->vtx_buffer);
			if (i->_priv->idx_buffer)
				graphics::destroy_buffer(d, i->_priv->idx_buffer);
#if defined(FLAME_GRAPHICS_VULKAN)
			graphics::destroy_sampler(d, i->_priv->font_sam);
			graphics::destroy_textureview(d, i->_priv->font_view);
#endif
			graphics::destroy_texture(d, i->_priv->font_tex);
#if defined(FLAME_GRAPHICS_VULKAN)
			d->dp->destroy_descriptorset(i->_priv->ds);
#endif
			graphics::destroy_shader(d, i->_priv->vert);
			graphics::destroy_shader(d, i->_priv->frag);
			graphics::destroy_pipeline(d, i->_priv->pl);

			delete i->_priv;
			delete i;
		}
	}
}

