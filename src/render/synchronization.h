#pragma once

#include "vulkan.h"

namespace tke
{
	VkFence createFence();
	void destroyFence(VkFence fence);

	VkEvent createEvent();
	void destroyEvent(VkEvent event);

	VkSemaphore createSemaphore();
	void destroySemaphore(VkSemaphore semaphore);

	void waitFence(VkFence);
}
