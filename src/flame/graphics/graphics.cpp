#include <vector>
#include <assert.h>

#include <flame/global.h>
#include <flame/system.h>

#include "graphics.h"
#include "graphics_private.h"

//#include <flame/global.h>
//#include <flame/engine/core/core.h>
//#include <flame/engine/graphics/graphics.h>
//#include <flame/engine/graphics/command_buffer.h>
//#include <flame/engine/graphics/buffer.h>
//#include <flame/engine/graphics/texture.h>
//#include <flame/engine/graphics/material.h>
//#include <flame/engine/graphics/descriptor.h>
//#include <flame/engine/graphics/renderpass.h>
//#include <flame/engine/graphics/pipeline.h>
//#include <flame/engine/graphics/shader.h>
//#include <flame/engine/graphics/sampler.h>
//#include <flame/engine/graphics/renderer.h>
//#include <flame/engine/graphics/pick_up.h>

namespace flame
{
	namespace graphics
	{
		void Graphics::set_resolution(int x, int y)
		{
			if (x != 0)
				resolution_x = x;
			if (y != 0)
				resolution_y = y;

			aspect = (float)resolution_x / resolution_y;

			_priv->resolution_change_event = true;
		}

		void *Graphics::add_resolution_change_listener(const std::function<void(int, int)> &e)
		{
			_priv->resolution_change_listeners.push_back(e);
			return &_priv->resolution_change_listeners.back();
		}

		void Graphics::remove_resolution_change_listener(void *p)
		{
			for (auto it = _priv->resolution_change_listeners.begin(); it != _priv->resolution_change_listeners.end(); it++)
			{
				if (&(*it) == p)
				{
					_priv->resolution_change_listeners.erase(it);
					return;
				}
			}
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
			int32_t messageCode,
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData)
		{
			//printf("\n=====VK ERROR=====\nERROR NUM:%d\n%s\n==================\n", messageCode, pMessage);

			if (messageCode == 8) return VK_FALSE; // Your computer is not support anisotropy, never mind
			if (messageCode == 10) return VK_FALSE; // Dest AccessMask 0 [None] must have required access bit 4096 [VK_ACCESS_TRANSFER_WRITE_BIT]  when layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, unless the app has previously added a barrier for this transition.
			if (messageCode == 0) return VK_FALSE; // descriptor set bind warmming
			if (messageCode == 2) return VK_FALSE; // Vertex attribute not consumed by vertex shader, never mind
			if (messageCode == 53) return VK_FALSE; // You have gave more clear values, never mind
			if (messageCode == 1) return VK_FALSE; // THREADING ERROR, 0.0 what is this

			if (messageCode == 54 || messageCode == 113246970) return VK_FALSE; // vkCreateDevice: pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex (= 0) is not less than any previously obtained pQueueFamilyPropertyCount from vkGetPhysicalDeviceQueueFamilyProperties (the pQueueFamilyPropertyCount was never obtained)
			if (messageCode == 5) return VK_FALSE; // SPIR-V module not valid: Operand 4 of MemberDecorate requires one of these capabilities: MultiViewport 
			if (messageCode == 13) return VK_FALSE; // Shader expects at least n descriptors but only less provided, never mind
			if (messageCode == 61) return VK_FALSE; // Some descriptor maybe used before any update, never mind
			if (messageCode == 15) return VK_FALSE; // Shader requires VkPhysicalDeviceFeatures::tessellationShader but is not enabled on the device, never mind

			if (messageCode == 52 || messageCode == 440402828) return VK_FALSE; // At Draw time the active render pass is incompatible w/ gfx pipeline

																				// ignore above

			if (messageCode == 6)
				return VK_FALSE;
			if (messageCode == 101) return VK_FALSE; // vkQueuePresentKHR: Presenting image without calling vkGetPhysicalDeviceSurfaceSupportKHR
			if (messageCode == 100) return VK_FALSE; // vkCreateSwapChainKHR(): surface capabilities not retrieved for this physical device
			if (messageCode == 1922 || messageCode == 341838316) return VK_FALSE; // vkCreateSwapChainKHR(): pCreateInfo->surface is not known at this time to be supported for presentation by this device. The vkGetPhysicalDeviceSurfaceSupportKHR() must be called beforehand, and it must return VK_TRUE support with this surface for at least one queue family of this device
			if (messageCode == 24) return VK_FALSE; // Vertex buffers are bound to command buffer but no vertex buffers are attached to this Pipeline State Object.
			if (messageCode == 59) return VK_FALSE; // Descriptor set encountered the following validation error at vkCmdDrawIndexed() time: Descriptor is being used in draw but has not been updated.
			if (messageCode == 63) return VK_FALSE; // vkBeginCommandBuffer(): Secondary Command Buffers may perform better if a valid framebuffer parameter is specified.
			if (messageCode == 14) return VK_FALSE;
			if (messageCode == 12) return VK_FALSE; // Push constant range covering variable starting at offset not accessible from stage
			if (messageCode == 4) return VK_FALSE;  //Pipeline needs renderpass information

			return VK_FALSE;
		}

		static VkPhysicalDeviceMemoryProperties memProperties;

		Graphics *create_graphics(bool enable_debug, int _resolution_x, int _resolution_y)
		{
			auto g = new Graphics;
			g->_priv = new GraphicsPrivate;

			g->near_plane = 0.1f;
			g->far_plane = 1000.f;
			g->fovy = 60.f;

			g->set_resolution(_resolution_x, _resolution_y);

			VkApplicationInfo appInfo;
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = nullptr;
			appInfo.pApplicationName = "";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Flame Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			std::vector<const char*> instLayers;
			if (enable_debug)
				instLayers.push_back("VK_LAYER_LUNARG_standard_validation");

			std::vector<const char*> instExtensions;
			instExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
			instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
			if (enable_debug)
				instExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			VkInstanceCreateInfo instInfo = {};
			instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instInfo.pApplicationInfo = &appInfo;
			instInfo.enabledExtensionCount = instExtensions.size();
			instInfo.ppEnabledExtensionNames = instExtensions.data();
			instInfo.enabledLayerCount = instLayers.size();
			instInfo.ppEnabledLayerNames = instLayers.data();
			vk_chk_res(vkCreateInstance(&instInfo, nullptr, &g->_priv->instance));

			if (enable_debug)
			{
				static auto _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(g->_priv->instance, "vkCreateDebugReportCallbackEXT"));

				VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
				callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
				callbackCreateInfo.pNext = nullptr;
				callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
				callbackCreateInfo.pfnCallback = &_vkDebugCallback;
				callbackCreateInfo.pUserData = nullptr;

				VkDebugReportCallbackEXT callback;
				vk_chk_res(_vkCreateDebugReportCallbackEXT(g->_priv->instance, &callbackCreateInfo, nullptr, &callback));
			}

			uint32_t gpuCount = 1;
			vk_chk_res(vkEnumeratePhysicalDevices(g->_priv->instance, &gpuCount, &g->_priv->physical_device));

			VkPhysicalDeviceFeatures features;

			vkGetPhysicalDeviceProperties(g->_priv->physical_device, &g->_priv->physical_device_properties);
			unsigned int queueFamilyPropertyCount = 0;
			std::vector<VkQueueFamilyProperties> queueFamilyProperties;
			vkGetPhysicalDeviceQueueFamilyProperties(g->_priv->physical_device, &queueFamilyPropertyCount, nullptr);
			queueFamilyProperties.resize(queueFamilyPropertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(g->_priv->physical_device, &queueFamilyPropertyCount, queueFamilyProperties.data());

			float queuePriorities[1] = {0.0};
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = 0;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = queuePriorities;

			std::vector<VkDeviceQueueCreateInfo> queueInfos;
			queueInfos.push_back(queueInfo);

			unsigned int extensionCount;

			vkGetPhysicalDeviceFeatures(g->_priv->physical_device, &g->_priv->physical_device_features);

			vkGetPhysicalDeviceMemoryProperties(g->_priv->physical_device, &memProperties);

			std::vector<char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
			VkDeviceCreateInfo deviceInfo = {};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.pQueueCreateInfos = queueInfos.data();
			deviceInfo.queueCreateInfoCount = queueInfos.size();
			deviceInfo.enabledExtensionCount = deviceExtensions.size();
			deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
			deviceInfo.pEnabledFeatures = &g->_priv->physical_device_features;
			vk_chk_res(vkCreateDevice(g->_priv->physical_device, &deviceInfo, nullptr, &g->_priv->device));

			//command_pool = new CommandPool;
			//descriptorPool = new DescriptorPool;

			//init_buffer();
			//init_texture();
			//init_sampler();
			//init_material();
			//init_shader();
			//init_pick_up();
			return g;
		}

		void destroy_graphics(Graphics *g)
		{

		}

		int vk_find_memory_type(uint typeFilter, VkMemoryPropertyFlags properties)
		{
			for (uint i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}
			return -1;
		}
	}

	VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

	void vk_device_wait_idle()
	{
		//vkDeviceWaitIdle(g->_priv->device);
	}

	void vk_queue_submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
	{
		//VkSubmitInfo info;
		//info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//info.pNext = nullptr;
		//VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//info.pWaitDstStageMask = &waitStage;
		//info.waitSemaphoreCount = waitSemaphore ? 1 : 0;
		//info.pWaitSemaphores = waitSemaphore ? &waitSemaphore : nullptr;
		//info.commandBufferCount = count;
		//info.pCommandBuffers = cmds;
		//info.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		//info.pSignalSemaphores = signalSemaphore ? &signalSemaphore : nullptr;

		//vk_chk_res(vkQueueSubmit(g->_priv->graphics_queue, 1, &info, VK_NULL_HANDLE));
	}

	void vk_queue_wait_idle()
	{
		//vk_chk_res(vkQueueWaitIdle(g->_priv->graphics_queue));
	}

	const char *vk_device_type_names[] = {
		"other",
		"integrated gpu",
		"discrete gpu",
		"virtual gpu",
		"cpu"
	};
}
