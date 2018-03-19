#include <assert.h>

#include <flame/engine/graphics/synchronization.h>

namespace flame
{
	VkFence createFence()
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence fence;
		vk_chk_res(vkCreateFence(vk_device, &info, nullptr, &fence));

		return fence;
	}

	void destroyFence(VkFence fence)
	{
		vkDestroyFence(vk_device, fence, nullptr);
	}

	VkEvent createEvent()
	{
		VkEventCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

		VkEvent event;
		vk_chk_res(vkCreateEvent(vk_device, &info, nullptr, &event));

		return event;
	}

	void destroyEvent(VkEvent event)
	{
		vkDestroyEvent(vk_device, event, nullptr);
	}

	VkSemaphore createSemaphore()
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		vk_chk_res(vkCreateSemaphore(vk_device, &info, nullptr, &semaphore));

		return semaphore;
	}

	void destroySemaphore(VkSemaphore semaphore)
	{
		vkDestroySemaphore(vk_device, semaphore, nullptr);
	}

	void wait_fence(VkFence fence)
	{
		vk_chk_res(vkWaitForFences(vk_device, 1, &fence, true, UINT64_MAX));
		vk_chk_res(vkResetFences(vk_device, 1, &fence));
	}
}
