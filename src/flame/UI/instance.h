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

#pragma once

#include <flame/math.h>
#include <flame/string.h>
#include <flame/surface.h>

#include "UI.h"

namespace flame
{
	struct Surface;
	
	namespace graphics
	{
		struct Textureview;
	}

	namespace UI
	{
		enum WindowFlag
		{
			WindowSaveSetting = 1 << 0,
			WindowNoResize = 1 << 1,
			WindowNoMove = 1 << 2
		};

		struct DrawlistPrivate;

		struct Drawlist
		{
			DrawlistPrivate *_priv;

			FLAME_UI_EXPORTS void push_cliprect(const Rect &rect);
			FLAME_UI_EXPORTS void pop_cliprect();
			FLAME_UI_EXPORTS void add_line(const Vec2 &a, const Vec2 &b, const Vec4 &col);
			FLAME_UI_EXPORTS void add_rect(const Rect &rect, const Vec4 &col, float round = 0.f, bool round_LT = true, bool round_RT = true, bool round_LB = true, bool round_RB = true);
			FLAME_UI_EXPORTS void add_rect_filled(const Rect &rect, const Vec4 &col, float round = 0.f, bool round_LT = true, bool round_RT = true, bool round_LB = true, bool round_RB = true);
			FLAME_UI_EXPORTS void add_circle(const Vec2 &center, float radius, const Vec4 &col);
			FLAME_UI_EXPORTS void add_circle_filled(const Vec2 &center, float radius, const Vec4 &col);
			FLAME_UI_EXPORTS void add_bezier(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec4 &col, float thickness);
			FLAME_UI_EXPORTS void add_text(const Vec2 &pos, const Vec4 &col, const char *fmt, ...);
			FLAME_UI_EXPORTS void add_image();
			FLAME_UI_EXPORTS void draw_grid(const Vec2 &off, const Vec2 &size);
		};

		struct InstancePrivate;

		struct Instance
		{
			bool processed_mouse_input;
			bool processed_keyboard_input;
			float elapsed_time;

			InstancePrivate *_priv;

			FLAME_UI_EXPORTS void set_texture(int index, graphics::Textureview *tv);

			FLAME_UI_EXPORTS void begin(int cx, int cy, float _elapsed_time);
			FLAME_UI_EXPORTS void end();
			FLAME_UI_EXPORTS void record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb);

			// * use inf to disable setting pos or/and size
			// flags - combination of WindowFlag
			FLAME_UI_EXPORTS bool begin_window(const char *name, const Vec2 &pos, const Vec2 &size, int flags = 0); 

			FLAME_UI_EXPORTS bool begin_plain_window(const char *name, const Vec2 &pos, const Vec2 &size);
			FLAME_UI_EXPORTS bool begin_status_window();
			FLAME_UI_EXPORTS void end_window();

			FLAME_UI_EXPORTS bool begin_mainmenu();
			FLAME_UI_EXPORTS void end_mainmenu();
			FLAME_UI_EXPORTS bool begin_menu(const char *label);
			FLAME_UI_EXPORTS void end_menu();
			FLAME_UI_EXPORTS bool menuitem(const char *label, const char *shortcut = nullptr, bool checked = false);

			FLAME_UI_EXPORTS void begin_tabbar(const char *str_id);
			FLAME_UI_EXPORTS void end_tabbar();
			FLAME_UI_EXPORTS bool tabitem(const char *label);

			FLAME_UI_EXPORTS bool begin_child(const char *str_id, const Vec2 &size, bool border);
			FLAME_UI_EXPORTS void end_child();

			FLAME_UI_EXPORTS void separator();
			FLAME_UI_EXPORTS bool button(const char *label);
			FLAME_UI_EXPORTS bool checkbox(const char *label, bool *p);
			FLAME_UI_EXPORTS bool dragfloat(const char *label, float *p, float speed, float v_min = 0.f, float v_max = 0.f);
			FLAME_UI_EXPORTS bool dragfloat2(const char *label, Vec2 *p, float speed, float v_min = 0.f, float v_max = 0.f);
			FLAME_UI_EXPORTS bool dragfloat3(const char *label, Vec3 *p, float speed, float v_min = 0.f, float v_max = 0.f);
			FLAME_UI_EXPORTS bool dragfloat4(const char *label, Vec4 *p, float speed, float v_min = 0.f, float v_max = 0.f);
			FLAME_UI_EXPORTS void text_unformatted(const char *text);
			FLAME_UI_EXPORTS void text_unformatted_RA(const char *text); // right align
			FLAME_UI_EXPORTS void text(const char *fmt, ...);
			FLAME_UI_EXPORTS void ID_text_unformatted(const char *str_id, const char *text);
			FLAME_UI_EXPORTS bool inputtext(const char *label, char *dst, int len);
			FLAME_UI_EXPORTS bool selectable(const char *label, bool selected);
			FLAME_UI_EXPORTS void image(int index, const Vec2 &size);
			FLAME_UI_EXPORTS void invisibleitem(const char *str_id, const Vec2 &size);

			FLAME_UI_EXPORTS Vec2 get_cursor_pos();
			FLAME_UI_EXPORTS void set_cursor_pos(const Vec2 &pos);

			FLAME_UI_EXPORTS void sameline();

			FLAME_UI_EXPORTS void push_item_width(float width);
			FLAME_UI_EXPORTS void pop_item_width();

			FLAME_UI_EXPORTS void push_ID(int ID);
			FLAME_UI_EXPORTS void pop_ID();

			FLAME_UI_EXPORTS unsigned int get_last_ID();
			FLAME_UI_EXPORTS bool is_last_item_focused();
			FLAME_UI_EXPORTS bool is_curr_window_focused();
			FLAME_UI_EXPORTS bool is_last_item_hovered();
			FLAME_UI_EXPORTS bool is_curr_window_hovered();
			FLAME_UI_EXPORTS bool is_last_item_active();
			FLAME_UI_EXPORTS Rect get_last_item_rect();
			FLAME_UI_EXPORTS Rect get_curr_window_rect();
			FLAME_UI_EXPORTS Rect get_curr_window_inner_rect();

			// return value - curr clip
			FLAME_UI_EXPORTS Rect set_global_cliprect(const Rect &rect);

			FLAME_UI_EXPORTS Drawlist get_overlap_drawlist();
			FLAME_UI_EXPORTS Drawlist get_curr_window_drawlist();

			FLAME_UI_EXPORTS void add_dialog(const char *title, int user_data_size, void *user_data,
				const std::function<void(Instance *ui, void *user_data, bool &out_open)> &show_callback);
			FLAME_UI_EXPORTS void add_message_dialog(const char *title, const char *message);
			FLAME_UI_EXPORTS void add_input_dialog(const char *title, const char *label, const 
				std::function<void(MediumString *input)> &callback, const char *default_input = nullptr);

			FLAME_UI_EXPORTS void set_mousecursor(CursorType type);
		};

		FLAME_UI_EXPORTS Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp, Surface *s);
		FLAME_UI_EXPORTS void destroy_instance(graphics::Device *d, Instance *i);
	}
}
