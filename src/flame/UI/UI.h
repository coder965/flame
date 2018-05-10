#pragma once

#ifdef _FLAME_UI_EXPORTS
#define FLAME_UI_EXPORTS __declspec(dllexport)
#else
#define FLAME_UI_EXPORTS __declspec(dllimport)
#endif

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Commandbuffer;
		struct Renderpass;
		struct Framebuffer;
	}

	namespace UI
	{
		struct InstancePrivate;

		struct Instance
		{
			bool processed_input;

			InstancePrivate *_priv;

			FLAME_UI_EXPORTS void begin(int cx, int cy, float elapsed_time, int mouse_x, int mouse_y,
				bool mouse_left_pressing, bool mouse_right_pressing, bool mouse_middle_pressing, int mouse_scroll);
			FLAME_UI_EXPORTS void end();
			FLAME_UI_EXPORTS void record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb);

			FLAME_UI_EXPORTS bool begin_window(const char *title);
			FLAME_UI_EXPORTS void end_window();
			FLAME_UI_EXPORTS bool button(const char *title);
			FLAME_UI_EXPORTS bool checkbox(const char *title, bool *p);
			FLAME_UI_EXPORTS bool dragfloat(const char *title, float *p, float speed);
			FLAME_UI_EXPORTS void text(const char *fmt, ...);
		};

		FLAME_UI_EXPORTS Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp);
		FLAME_UI_EXPORTS void destroy_instance(graphics::Device *d, Instance *i);
	}
}

