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
		struct Descriptorpool;
		struct Commandbuffer;
		struct Commandpool;
		struct Queue;
		struct Renderpass;
		struct Framebuffer;
		struct Texture;
	}

	namespace UI
	{
		struct InstancePrivate;

		struct Instance
		{
			InstancePrivate *_priv;

			FLAME_UI_EXPORTS void begin(int cx, int cy, float elapsed_time, int mouse_x, int mouse_y,
				bool mouse_left_pressing, bool mouse_right_pressing, bool mouse_middle_pressing, int mouse_scroll);
			FLAME_UI_EXPORTS void end(bool *out_need_update_commandbuffer);
			FLAME_UI_EXPORTS void record_commandbuffer(graphics::Commandbuffer *cb, graphics::Renderpass *rp, graphics::Framebuffer *fb);

			FLAME_UI_EXPORTS bool button(const char *title);
		};

		FLAME_UI_EXPORTS Instance *create_instance(graphics::Device *d, graphics::Renderpass *rp, graphics::Descriptorpool *dp, graphics::Commandpool *cp, graphics::Queue *q);
		FLAME_UI_EXPORTS void destroy_instance(graphics::Device *d, Instance *i);
	}
}

