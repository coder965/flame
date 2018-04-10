#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct QueuePrivate;

		struct Queue
		{
			QueuePrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void submit();
		};

		FLAME_GRAPHICS_EXPORTS Queue *create_queue(Graphics *g);
		FLAME_GRAPHICS_EXPORTS void destroy_queue(Graphics *g, Queue *q);
	}
}
