#include <assert.h>

#include "vulkan.h"

namespace tke
{
	void Device::waitIdle()
	{
		mtx.lock();
		vkDeviceWaitIdle(v);
		mtx.unlock();
	}

	void Queue::waitIdle()
	{
		mtx.lock();
		vkQueueWaitIdle(v);
		mtx.unlock();
	}

	void Queue::submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
	{
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;
		info.waitSemaphoreCount = waitSemaphore ? 1 : 0;
		info.pWaitSemaphores = &waitSemaphore;
		info.commandBufferCount = count;
		info.pCommandBuffers = cmds;
		info.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		info.pSignalSemaphores = &signalSemaphore;

		graphicsQueue.mtx.lock();
		auto res = vkQueueSubmit(graphicsQueue.v, 1, &info, fence);
		assert(res == VK_SUCCESS);
		graphicsQueue.mtx.unlock();
	}

	Instance inst;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	Device device;
	Queue graphicsQueue;
	static VkPhysicalDeviceMemoryProperties memProperties;

	int findVkMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		return -1;
	}

	void initRenderThreadData()
	{

	}

	void initRender()
	{

	}
}
