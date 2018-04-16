#pragma once

#include "queue.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct QueuePrivate
		{
			Device *d;
			VkQueue v;
		};
	}
}