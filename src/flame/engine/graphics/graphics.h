#pragma once

#include <mutex>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#undef INFINITE

#include <flame/global.h>

namespace flame
{
	extern float near_plane;
	extern float far_plane;
	extern float fovy;

	enum DescriptorSetBindings
	{
		MainDescriptorSetBinding,
		MaterialDescriptorSetBinding,
		BoneSetDescriptorBinding
	};

	enum MainDescriptorSetBindings
	{
		ConstantBufferDescriptorBinding,
		MatrixBufferDescriptorBinding,
		ObjectMatrixBufferDescriptorBinding,
		TerrainBufferDescriptorBinding,
		TerrainBlendImageDescriptorBinding,
		WaterBufferDescriptorBinding,
		DepthImageDescriptorBinding,
		AlbedoAlphaImageDescriptorBinding,
		NormalHeightImageDescriptorBinding,
		SpecRoughnessDescriptorImageBinding,
		LightBufferDescriptorBinding,
		EnvrImageDescriptorBinding,
		AmbientBufferDescriptorBinding,
		AoImageDescriptorBinding,
		ShadowBufferDescriptorBinding,
		ShadowImageDescriptorBinding
	};

	enum MaterialDescriptorSetBindings
	{
		MaterialBufferDescriptorBinding,
		MaterialImagesDescriptorBinding
	};

	const VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

	struct Device
	{
		VkDevice v;
		std::mutex mtx;

		void wait_idle();
	};

	struct Queue
	{
		VkQueue v;
		std::mutex mtx;

		void wait_idle();
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

	int find_vk_memory_type(uint typeFilter, VkMemoryPropertyFlags properties);

	void init_graphics(bool debug, int _resolution_x, int _resolution_y, bool watch_shader_file);
}
