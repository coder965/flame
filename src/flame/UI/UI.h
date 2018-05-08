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
		struct Framebuffer;
		struct Texture;
	}

	namespace UI
	{
		struct InstancePrivate;

		struct Instance
		{
			InstancePrivate *_priv;

			FLAME_UI_EXPORTS void begin();
			FLAME_UI_EXPORTS void end();
			FLAME_UI_EXPORTS void record_commandbuffer(graphics::Commandbuffer *cb);
		};

		FLAME_UI_EXPORTS Instance *create_instance(graphics::Device *d, graphics::Descriptorpool *dp, graphics::Commandpool *cp, graphics::Queue *q);
		FLAME_UI_EXPORTS void destroy_instance(graphics::Device *d, Instance *i);
	}
}

