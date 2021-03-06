//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <vector>

#include <flame/type.h>

#include "device_private.h"
#include "commandbuffer_private.h"
#include "queue_private.h"
#include "descriptor_private.h"

namespace flame
{
	namespace graphics
	{
		Device *create_device(bool enable_debug)
		{
			auto d = new Device;

#if defined(FLAME_GRAPHICS_VULKAN)
			d->_priv = new DevicePrivate;

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
			vk_chk_res(vkCreateInstance(&instInfo, nullptr, &d->_priv->instance));

			if (enable_debug)
			{
				static auto _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(d->_priv->instance, "vkCreateDebugReportCallbackEXT"));

				VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
				callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
				callbackCreateInfo.pNext = nullptr;
				callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
				callbackCreateInfo.pfnCallback = 
					[](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
					int32_t messageCode,
					const char* pLayerPrefix,
					const char* pMessage,
					void* pUserData) -> VkBool32
				{
					printf("\n=====VK ERROR=====\nERROR NUM:%d\n%s\n==================\n", messageCode, pMessage);

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
				};
				callbackCreateInfo.pUserData = nullptr;

				VkDebugReportCallbackEXT callback;
				vk_chk_res(_vkCreateDebugReportCallbackEXT(d->_priv->instance, &callbackCreateInfo, nullptr, &callback));
			}

			uint32_t gpuCount = 1;
			vk_chk_res(vkEnumeratePhysicalDevices(d->_priv->instance, &gpuCount, &d->_priv->physical_device));

			VkPhysicalDeviceFeatures features;

			vkGetPhysicalDeviceProperties(d->_priv->physical_device, &d->_priv->physical_device_properties);
			unsigned int queueFamilyPropertyCount = 0;
			std::vector<VkQueueFamilyProperties> queueFamilyProperties;
			vkGetPhysicalDeviceQueueFamilyProperties(d->_priv->physical_device, &queueFamilyPropertyCount, nullptr);
			queueFamilyProperties.resize(queueFamilyPropertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(d->_priv->physical_device, &queueFamilyPropertyCount, queueFamilyProperties.data());

			float queuePriorities[1] = {0.0};
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = 0;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = queuePriorities;

			std::vector<VkDeviceQueueCreateInfo> queueInfos;
			queueInfos.push_back(queueInfo);

			unsigned int extensionCount;

			vkGetPhysicalDeviceFeatures(d->_priv->physical_device, &d->_priv->physical_device_features);

			vkGetPhysicalDeviceMemoryProperties(d->_priv->physical_device, &d->_priv->mem_properties);

			std::vector<char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
			VkDeviceCreateInfo deviceInfo = {};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.pQueueCreateInfos = queueInfos.data();
			deviceInfo.queueCreateInfoCount = queueInfos.size();
			deviceInfo.enabledExtensionCount = deviceExtensions.size();
			deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
			deviceInfo.pEnabledFeatures = &d->_priv->physical_device_features;
			vk_chk_res(vkCreateDevice(d->_priv->physical_device, &deviceInfo, nullptr, &d->_priv->device));

			d->features_texture_compression_BC = d->_priv->physical_device_features.textureCompressionBC;
			d->features_texture_compression_ASTC = d->_priv->physical_device_features.textureCompressionASTC_LDR;
			d->features_texture_compression_ETC2 = d->_priv->physical_device_features.textureCompressionETC2;

			d->cp = create_commandpool(d);
			d->q = create_queue(d);
			d->dp = create_descriptorpool(d);
#endif

			return d;
		}

		void destroy_device(Device *d)
		{

		}
	}
}

