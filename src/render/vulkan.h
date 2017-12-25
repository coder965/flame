#pragma once

#include <mutex>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include "../../VulkanSDK/1.0.65.1/Include/vulkan/vulkan.h"

namespace tke
{
	const VkFormat swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

	struct Device
	{
		VkDevice v;
		std::mutex mtx;

		void waitIdle();
	};

	struct Queue
	{
		VkQueue v;
		std::mutex mtx;

		void waitIdle();
		void submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore = 0, VkSemaphore signalSemaphore = 0, VkFence fence = 0);
	};

	extern VkInstance vk_instance;
	extern VkPhysicalDevice physicalDevice;
	extern VkPhysicalDeviceProperties physicalDeviceProperties;
	extern VkPhysicalDeviceFeatures physicalDeviceFeatures;
	extern Device vk_device;
	extern Queue graphicsQueue;

	int findVkMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	int initVulkan(bool debug);
}
