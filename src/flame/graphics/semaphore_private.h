#pragma once

#include "semaphore.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct SemaphorePrivate
		{
			VkSemaphore v;
		};
	}
}
