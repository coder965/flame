#pragma once

#include <flame/math.h>

#include "UI.h"

namespace flame
{
	namespace UI
	{
		struct InstancePrivate;

		struct Instance
		{
			bool processed_input;
			float elapsed_time;

			InstancePrivate *_priv;

			FLAME_UI_EXPORTS void begin(int cx, int cy, float _elapsed_time, int mouse_x, int mouse_y,
				bool mouse_left_pressing, bool mouse_right_pressing, bool mouse_middle_pressing, int mouse_scroll);
			FLAME_UI_EXPORTS void end();
			FLAME_UI_EXPORTS void record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb);

			// use inf to disable setting pos or/and size
			FLAME_UI_EXPORTS bool begin_window(const char *title, const glm::vec2 &pos, const glm::vec2 &size, bool need_save_setting = false); 

			FLAME_UI_EXPORTS bool begin_plain_window(const char *title, const glm::vec2 &pos, const glm::vec2 &size);
			FLAME_UI_EXPORTS void end_window();
			FLAME_UI_EXPORTS bool button(const char *title);
			FLAME_UI_EXPORTS bool checkbox(const char *title, bool *p);
			FLAME_UI_EXPORTS bool dragfloat(const char *title, float *p, float speed);
			FLAME_UI_EXPORTS bool dragfloat2(const char *title, glm::vec2 *p, float speed);
			FLAME_UI_EXPORTS bool dragfloat3(const char *title, glm::vec3 *p, float speed);
			FLAME_UI_EXPORTS bool dragfloat4(const char *title, glm::vec4 *p, float speed);
			FLAME_UI_EXPORTS void text(const char *fmt, ...);
			FLAME_UI_EXPORTS bool inputtext(const char *title, char *dst, int len);

			FLAME_UI_EXPORTS unsigned int get_last_ID();
			FLAME_UI_EXPORTS bool is_last_item_focused();
			FLAME_UI_EXPORTS bool is_curr_window_focused();
			FLAME_UI_EXPORTS bool is_last_item_hovered();
			FLAME_UI_EXPORTS bool is_curr_window_hovered();
			FLAME_UI_EXPORTS glm::vec4 get_last_item_rect();
			FLAME_UI_EXPORTS glm::vec4 get_curr_window_rect();
			FLAME_UI_EXPORTS void add_line_to_window(const glm::vec2 &a, const glm::vec2 &b, const glm::vec4 &col);
			FLAME_UI_EXPORTS void add_rect_to_window(const glm::vec4 &rect, const glm::vec4 &col);
			FLAME_UI_EXPORTS void add_text_to_window(const glm::vec2 &pos, const glm::vec4 &col, const char *fmt, ...);
			FLAME_UI_EXPORTS void add_rect_to_overlap(const glm::vec4 &rect, const glm::vec4 &col);
		};

		FLAME_UI_EXPORTS Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp);
		FLAME_UI_EXPORTS void destroy_instance(graphics::Device *d, Instance *i);
	}
}
