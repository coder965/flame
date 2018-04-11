#pragma once

#include <flame/global.h>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Commandbuffer;
		struct Swapchain;
		struct Semaphore;

		struct QueuePrivate;

		struct Queue
		{
			QueuePrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void submit(Commandbuffer *c, Semaphore *wait_semaphore, Semaphore *signal_semaphore);
			FLAME_GRAPHICS_EXPORTS void present(uint index, Swapchain *s, Semaphore *wait_semaphore);
		};

		FLAME_GRAPHICS_EXPORTS Queue *create_queue(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_queue(Device *d, Queue *q);
	}
}
