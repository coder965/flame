#include <assert.h>

#include <flame/engine/graphics/synchronization.h>

namespace flame
{
	VkFence createFence()
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence fence;
		auto res = vkCreateFence(vk_device.v, &info, nullptr, &fence);
		assert(res == VK_SUCCESS);

		return fence;
	}

	void destroyFence(VkFence fence)
	{
		vkDestroyFence(vk_device.v, fence, nullptr);
	}

	VkEvent createEvent()
	{
		VkEventCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

		VkEvent event;
		auto res = vkCreateEvent(vk_device.v, &info, nullptr, &event);
		assert(res == VK_SUCCESS);

		return event;
	}

	void destroyEvent(VkEvent event)
	{
		vkDestroyEvent(vk_device.v, event, nullptr);
	}

	VkSemaphore createSemaphore()
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		auto res = vkCreateSemaphore(vk_device.v, &info, nullptr, &semaphore);
		assert(res == VK_SUCCESS);

		return semaphore;
	}

	void destroySemaphore(VkSemaphore semaphore)
	{
		vkDestroySemaphore(vk_device.v, semaphore, nullptr);
	}

	void wait_fence(VkFence fence)
	{
		auto res = vkWaitForFences(vk_device.v, 1, &fence, true, UINT64_MAX);
		assert(res == VK_SUCCESS);
		res = vkResetFences(vk_device.v, 1, &fence);
		assert(res == VK_SUCCESS);
	}
}
