#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct SemaphorePrivate;

		struct Semaphore
		{
			SemaphorePrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Semaphore *create_semaphore(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_semaphore(Device *d, Semaphore *s);
	}
}

