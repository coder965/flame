#pragma once

#include "graphics.h"

namespace tke
{
	// must call in main thread
	VkFence createFence();
	// must call in main thread
	void destroyFence(VkFence fence);

	// must call in main thread
	VkEvent createEvent();
	// must call in main thread
	void destroyEvent(VkEvent event);

	// must call in main thread
	VkSemaphore createSemaphore();
	// must call in main thread
	void destroySemaphore(VkSemaphore semaphore);

	// must call in main thread
	void wait_fence(VkFence);
}
