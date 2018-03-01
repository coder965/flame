#pragma once

#include <mutex>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace flame
{
	const VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

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

	void chk_vk_res(VkResult res);

	extern const char *vk_device_type_names[];

	extern VkInstance vk_instance;
	extern VkPhysicalDevice vk_physical_device;
	extern VkPhysicalDeviceProperties vk_physical_device_properties;
	extern VkPhysicalDeviceFeatures vk_physical_device_features;
	extern Device vk_device;
	extern Queue vk_graphics_queue;

	int find_vk_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void init_graphics(bool debug, int _resolution_x, int _resolution_y, bool watch_shader_file);
}
