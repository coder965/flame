#pragma once

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#undef INFINITE

#include <flame/global.h>
#include <flame/engine/core/object.h>

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

	extern VkFormat swapchain_format;

	void vk_chk_res(VkResult res);

	extern const char *vk_device_type_names[];

	extern VkInstance vk_instance;
	extern VkPhysicalDevice vk_physical_device;
	extern VkPhysicalDeviceProperties vk_physical_device_properties;
	extern VkPhysicalDeviceFeatures vk_physical_device_features;
	extern VkDevice vk_device;
	extern VkQueue vk_graphics_queue;

	void vk_device_wait_idle();
	void vk_queue_submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore = 0, VkSemaphore signalSemaphore = 0, VkFence fence = 0);
	void vk_queue_wait_idle();

	enum FormatType
	{
		FormatTypeColor,
		FormatTypeDepth,
		FormatTypeDepthStencil
	};

	FormatType get_format_type(VkFormat format);

	int vk_find_memory_type(uint typeFilter, VkMemoryPropertyFlags properties);

	void init_graphics(bool debug, int _resolution_x, int _resolution_y);

	class Resolution : public Object
	{
	private:
		int _x, _y;
		float _aspect;

		long long _dirty_frame;
	public:
		int x() const;
		int y() const;
		float aspect() const;
		long long dirty_frame() const;

		void set(int x, int y);
		void set_x(int x);
		void set_y(int y);
	};

	extern Resolution resolution;

	enum LightingModel
	{
		LightingModelPhong,
		LightingModelPbr,
		LightingModelPbrIbl
	};
}
