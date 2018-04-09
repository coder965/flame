#pragma once

#ifdef _FLAME_GRAPHICS_EXPORTS
#define FLAME_GRAPHICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_GRAPHICS_EXPORTS __declspec(dllimport)
#endif

#include <functional>

//#include <flame/global.h>

namespace flame
{
	namespace graphics
	{
		struct Format
		{
			enum Value
			{
				R8,
				R8G8B8A8,
				R16G16B16A16,
				Swapchain,
				Depth16,
				Depth32,
				Depth24Stencil8
			};

			enum Type
			{
				TypeColor,
				TypeDepth,
				TypeDepthStencil
			};

			Value v;

			Type get_type()
			{
				if (v == Depth16 || v == Depth32)
					return TypeDepth;
				if (v == Depth24Stencil8)
					return TypeDepthStencil;
				return TypeColor;
			}
		};

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

		struct SwapchainPrivate;

		struct Swapchain
		{
			SwapchainPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS int acquire_image();
		};

		struct FramebufferPrivate;

		struct Framebuffer
		{
			FramebufferPrivate *_priv;
		};

		struct GraphicsPrivate;

		struct Graphics
		{
			float near_plane;
			float far_plane;
			float fovy;

			int resolution_x;
			int resolution_y;
			float aspect;

			GraphicsPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void set_resolution(int x, int y); // 0 means keep
			FLAME_GRAPHICS_EXPORTS void *add_resolution_change_listener(const std::function<void(int, int)> &e);
			FLAME_GRAPHICS_EXPORTS void remove_resolution_change_listener(void *p);

			FLAME_GRAPHICS_EXPORTS Swapchain *create_swapchain(void *, int cx, int cy);
			FLAME_GRAPHICS_EXPORTS void destroy_swapchain(Swapchain *s);
		};

		FLAME_GRAPHICS_EXPORTS Graphics *create_graphics(bool enable_debug, int _resolution_x, int _resolution_y);
		FLAME_GRAPHICS_EXPORTS void destroy_graphics(Graphics *g);
	}

	//extern VkFormat swapchain_format;

	//void vk_chk_res(VkResult res);

	//extern const char *vk_device_type_names[];

	//extern VkInstance vk_instance;
	//extern VkPhysicalDevice vk_physical_device;
	//extern VkPhysicalDeviceProperties vk_physical_device_properties;
	//extern VkPhysicalDeviceFeatures vk_physical_device_features;
	//extern VkDevice vk_device;
	//extern VkQueue vk_graphics_queue;

	//void vk_device_wait_idle();
	//void vk_queue_submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore = 0, VkSemaphore signalSemaphore = 0);
	//void vk_queue_wait_idle();

	//int vk_find_memory_type(uint typeFilter, VkMemoryPropertyFlags properties);

	enum LightingModel
	{
		LightingModelPhong,
		LightingModelPbr,
		LightingModelPbrIbl
	};
}
