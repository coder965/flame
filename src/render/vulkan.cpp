#include <assert.h>

#include "../utils.h"
#include "vulkan.h"
#include "command_buffer.h"
#include "descriptor.h"
#include "sampler.h"

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
		info.pWaitSemaphores = waitSemaphore ? &waitSemaphore : nullptr;
		info.commandBufferCount = count;
		info.pCommandBuffers = cmds;
		info.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		info.pSignalSemaphores = signalSemaphore ? &signalSemaphore : nullptr;

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

	void initVulkanThreadData()
	{

	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		if (messageCode == 0) return VK_FALSE; // descriptor set bind warmming
		if (messageCode == 8) return VK_FALSE; // Your fucking computer is not support anisotropy, never mind
		if (messageCode == 2) return VK_FALSE; // Vertex attribute not consumed by vertex shader, never mind
		if (messageCode == 6) return VK_FALSE; // Image layout should be attachment optimal but got general, never mind
		if (messageCode == 53) return VK_FALSE; // You have gave more clear values, never mind
		if (messageCode == 1) return VK_FALSE; // THREADING ERROR, 0.0 what is this

		if (messageCode == 54 || messageCode == 113246970) return VK_FALSE; // vkCreateDevice: pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex (= 0) is not less than any previously obtained pQueueFamilyPropertyCount from vkGetPhysicalDeviceQueueFamilyProperties (the pQueueFamilyPropertyCount was never obtained)
		if (messageCode == 5) return VK_FALSE; // SPIR-V module not valid: Operand 4 of MemberDecorate requires one of these capabilities: MultiViewport 
		if (messageCode == 13) return VK_FALSE; // Shader expects at least n descriptors but only less provided, never mind
		if (messageCode == 61) return VK_FALSE; // Some descriptor maybe used before any update, never mind
		if (messageCode == 15) return VK_FALSE; // Shader requires VkPhysicalDeviceFeatures::tessellationShader but is not enabled on the device, never mind

		if (messageCode == 52 || messageCode == 440402828) return VK_FALSE; // At Draw time the active render pass is incompatible w/ gfx pipeline

												// ignore above

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

	int initVulkan(bool debug)
	{
		VkResult res;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "TK Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "TK Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> instLayers;
		if (debug) instLayers.push_back("VK_LAYER_LUNARG_standard_validation");

		std::vector<const char*> instExtensions;
		instExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		if (debug) instExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		VkInstanceCreateInfo instInfo = {};
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pApplicationInfo = &appInfo;
		instInfo.enabledExtensionCount = instExtensions.size();
		instInfo.ppEnabledExtensionNames = instExtensions.data();
		instInfo.enabledLayerCount = instLayers.size();
		instInfo.ppEnabledLayerNames = instLayers.data();
		res = vkCreateInstance(&instInfo, nullptr, &inst.v);
		if (res != VkResult::VK_SUCCESS)
			return ErrContextLost;

		if (debug)
		{
			static auto _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(inst.v, "vkCreateDebugReportCallbackEXT"));

			VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
			callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			callbackCreateInfo.pNext = nullptr;
			callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			callbackCreateInfo.pfnCallback = &_vkDebugCallback;
			callbackCreateInfo.pUserData = nullptr;

			VkDebugReportCallbackEXT callback;
			res = _vkCreateDebugReportCallbackEXT(inst.v, &callbackCreateInfo, nullptr, &callback);
			assert(res == VK_SUCCESS);
		}

		uint32_t gpuCount = 1;
		res = vkEnumeratePhysicalDevices(inst.v, &gpuCount, &physicalDevice);
		if (res != VkResult::VK_SUCCESS)
			return ErrContextLost;

		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		unsigned int queueFamilyPropertyCount = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
		queueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		float queuePriorities[1] = { 0.0 };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = 0;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = queuePriorities;

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		queueInfos.push_back(queueInfo);

		std::vector<char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.queueCreateInfoCount = queueInfos.size();
		deviceInfo.enabledExtensionCount = deviceExtensions.size();
		deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
		res = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device.v);
		assert(res == VK_SUCCESS);

		vkGetDeviceQueue(device.v, 0, 0, &graphicsQueue.v);

		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		commandPool = new CommandPool;
		descriptorPool = new DescriptorPool;

		initPipeline();
		initSampler();

		return NoErr;
	}
}
