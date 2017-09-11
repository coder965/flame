#include <assert.h>

#include "synchronization.h"

namespace tke
{
	VkFence createFence()
	{
		VkFence fence;

		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		device.mtx.lock();
		auto res = vkCreateFence(device.v, &info, nullptr, &fence);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		return fence;
	}

	void destroyFence(VkFence fence)
	{
		device.mtx.lock();
		vkDestroyFence(device.v, fence, nullptr);
		device.mtx.unlock();
	}

	VkEvent createEvent()
	{
		VkEvent event;

		VkEventCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

		device.mtx.lock();
		auto res = vkCreateEvent(device.v, &info, nullptr, &event);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		return event;
	}

	void destroyEvent(VkEvent event)
	{
		device.mtx.lock();
		vkDestroyEvent(device.v, event, nullptr);
		device.mtx.unlock();
	}

	VkSemaphore createSemaphore()
	{
		VkSemaphore semaphore;

		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		device.mtx.lock();
		auto res = vkCreateSemaphore(device.v, &info, nullptr, &semaphore);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		return semaphore;
	}

	void destroySemaphore(VkSemaphore semaphore)
	{
		device.mtx.lock();
		vkDestroySemaphore(device.v, semaphore, nullptr);
		device.mtx.unlock();
	}

	void waitFence(VkFence fence)
	{
		device.mtx.lock();
		VkResult res;
		res = vkWaitForFences(device.v, 1, &fence, true, UINT64_MAX);
		assert(res == VK_SUCCESS);
		res = vkResetFences(device.v, 1, &fence);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}
}
