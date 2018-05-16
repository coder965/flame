#pragma once

#include <flame/math.h>
#include <flame/string.h>
#include <flame/surface.h>

#include "UI.h"

namespace flame
{
	struct Surface;

	namespace UI
	{
		enum WindowFlag
		{
			WindowSaveSetting = 1 << 0,
			WindowNoResize = 1 << 1,
			WindowNoMove = 1 << 2
		};

		struct InstancePrivate;

		struct Instance
		{
			bool processed_input;
			float elapsed_time;

			InstancePrivate *_priv;

			FLAME_UI_EXPORTS void begin(int cx, int cy, float _elapsed_time);
			FLAME_UI_EXPORTS void end();
			FLAME_UI_EXPORTS void record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb);

			// * use inf to disable setting pos or/and size
			// flags - combination of WindowFlag
			FLAME_UI_EXPORTS bool begin_window(const char *name, const glm::vec2 &pos, const glm::vec2 &size, int flags = 0); 

			FLAME_UI_EXPORTS bool begin_plain_window(const char *name, const glm::vec2 &pos, const glm::vec2 &size);
			FLAME_UI_EXPORTS bool begin_status_window();
			FLAME_UI_EXPORTS void end_window();

			FLAME_UI_EXPORTS bool begin_mainmenu();
			FLAME_UI_EXPORTS void end_mainmenu();
			FLAME_UI_EXPORTS bool begin_menu(const char *label);
			FLAME_UI_EXPORTS void end_menu();
			FLAME_UI_EXPORTS bool menuitem(const char *label);

			FLAME_UI_EXPORTS bool button(const char *label);
			FLAME_UI_EXPORTS bool checkbox(const char *label, bool *p);
			FLAME_UI_EXPORTS bool dragfloat(const char *label, float *p, float speed);
			FLAME_UI_EXPORTS bool dragfloat2(const char *label, glm::vec2 *p, float speed);
			FLAME_UI_EXPORTS bool dragfloat3(const char *label, glm::vec3 *p, float speed);
			FLAME_UI_EXPORTS bool dragfloat4(const char *label, glm::vec4 *p, float speed);
			FLAME_UI_EXPORTS void text_unformatted(const char *text);
			FLAME_UI_EXPORTS void text(const char *fmt, ...);
			FLAME_UI_EXPORTS void ID_text_unformatted(const char *ID, const char *text);
			FLAME_UI_EXPORTS bool inputtext(const char *label, char *dst, int len);
			FLAME_UI_EXPORTS bool selectable(const char *label, bool selected);

			FLAME_UI_EXPORTS unsigned int get_last_ID();
			FLAME_UI_EXPORTS bool is_last_item_focused();
			FLAME_UI_EXPORTS bool is_curr_window_focused();
			FLAME_UI_EXPORTS bool is_last_item_hovered();
			FLAME_UI_EXPORTS bool is_curr_window_hovered();
			FLAME_UI_EXPORTS glm::vec4 get_last_item_rect();
			FLAME_UI_EXPORTS glm::vec4 get_curr_window_rect();

			// only one layer
			FLAME_UI_EXPORTS void push_displayrect(const glm::vec4 &rect);

			FLAME_UI_EXPORTS void pop_displayrect();
			FLAME_UI_EXPORTS void push_overlay_cliprect(const glm::vec4 &rect);
			FLAME_UI_EXPORTS void pop_overlay_cliprect();
			FLAME_UI_EXPORTS void add_line_to_window(const glm::vec2 &a, const glm::vec2 &b, const glm::vec4 &col);
			FLAME_UI_EXPORTS void add_rect_to_window(const glm::vec4 &rect, const glm::vec4 &col);
			FLAME_UI_EXPORTS void add_text_to_window(const glm::vec2 &pos, const glm::vec4 &col, const char *fmt, ...);
			FLAME_UI_EXPORTS void add_rect_to_overlap(const glm::vec4 &rect, const glm::vec4 &col);

			FLAME_UI_EXPORTS void add_message_dialog(const char *title, const char *message);
			FLAME_UI_EXPORTS void add_input_dialog(const char *title, const char *label, const 
				std::function<void(MediumString *input)> &callback, const char *default_input = nullptr);

			FLAME_UI_EXPORTS void set_cursor(CursorType type);
		};

		FLAME_UI_EXPORTS Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp, Surface *s);
		FLAME_UI_EXPORTS void destroy_instance(graphics::Device *d, Instance *i);
	}
}
