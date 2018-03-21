#include <assert.h>
#include <set>

#include <flame/global.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/graphics.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/material.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/pipeline.h>
#include <flame/engine/graphics/shader.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/graphics/pick_up.h>

namespace flame
{
	float near_plane = 0.1f;
	float far_plane = 1000.f;
	float fovy = 60.f;

	VkInstance vk_instance;
	VkPhysicalDevice vk_physical_device;
	VkPhysicalDeviceProperties vk_physical_device_properties;
	VkPhysicalDeviceFeatures vk_physical_device_features;
	VkDevice vk_device;
	VkQueue vk_graphics_queue;

	void vk_chk_res(VkResult res)
	{
		assert(res == VK_SUCCESS);
	}

	void vk_device_wait_idle()
	{
		vkDeviceWaitIdle(vk_device);
	}

	void vk_queue_submit(int count, VkCommandBuffer *cmds, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
	{
		VkSubmitInfo info;
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;
		info.waitSemaphoreCount = waitSemaphore ? 1 : 0;
		info.pWaitSemaphores = waitSemaphore ? &waitSemaphore : nullptr;
		info.commandBufferCount = count;
		info.pCommandBuffers = cmds;
		info.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		info.pSignalSemaphores = signalSemaphore ? &signalSemaphore : nullptr;

		vk_chk_res(vkQueueSubmit(vk_graphics_queue, 1, &info, fence));
	}

	void vk_queue_wait_idle()
	{
		vk_chk_res(vkQueueWaitIdle(vk_graphics_queue));
	}

	const char *vk_device_type_names[] = {
		"other",
		"integrated gpu",
		"discrete gpu",
		"virtual gpu",
		"cpu"
	};

	FormatType get_format_type(VkFormat format)
	{
		if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT)
			return FormatTypeDepth;
		if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			return FormatTypeDepthStencil;
		return FormatTypeColor;
	}

	static VkPhysicalDeviceMemoryProperties memProperties;

	int vk_find_memory_type(uint typeFilter, VkMemoryPropertyFlags properties)
	{
		for (uint i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		return -1;
	}

	struct VulkanError
	{
		VkDebugReportObjectTypeEXT objectType;
		uint64_t object;
		size_t location;
		int32_t messageCode;
	};

	bool operator<(const VulkanError &a, const VulkanError &b)
	{
		return a.messageCode < b.messageCode;
	}

	std::set<VulkanError> _vulkan_errors;

	static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		if (messageCode == 6)
			int cut = 1;

		auto pr = _vulkan_errors.insert({objectType, object, location, messageCode});
		if (pr.second)
			printf("\n=====VK ERROR=====\nERROR NUM:%d\n%s\n==================\n", messageCode, pMessage);

		if (messageCode == 8) return VK_FALSE; // Your computer is not support anisotropy, never mind
		if (messageCode == 10) return VK_FALSE; // Dest AccessMask 0 [None] must have required access bit 4096 [VK_ACCESS_TRANSFER_WRITE_BIT]  when layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, unless the app has previously added a barrier for this transition.
		if (messageCode == 0) return VK_FALSE; // descriptor set bind warmming
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

	void init_graphics(bool debug, int _resolution_x, int _resolution_y)
	{
		resolution.set(_resolution_x, _resolution_y);

		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Flame Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> instLayers;
		if (debug) 
			instLayers.push_back("VK_LAYER_LUNARG_standard_validation");

		std::vector<const char*> instExtensions;
		instExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		if (debug) 
			instExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		VkInstanceCreateInfo instInfo = {};
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pApplicationInfo = &appInfo;
		instInfo.enabledExtensionCount = instExtensions.size();
		instInfo.ppEnabledExtensionNames = instExtensions.data();
		instInfo.enabledLayerCount = instLayers.size();
		instInfo.ppEnabledLayerNames = instLayers.data();
		vk_chk_res(vkCreateInstance(&instInfo, nullptr, &vk_instance));

		if (debug)
		{
			static auto _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugReportCallbackEXT"));

			VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
			callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			callbackCreateInfo.pNext = nullptr;
			callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			callbackCreateInfo.pfnCallback = &_vkDebugCallback;
			callbackCreateInfo.pUserData = nullptr;

			VkDebugReportCallbackEXT callback;
			vk_chk_res(_vkCreateDebugReportCallbackEXT(vk_instance, &callbackCreateInfo, nullptr, &callback));
		}

		uint32_t gpuCount = 1;
		vk_chk_res(vkEnumeratePhysicalDevices(vk_instance, &gpuCount, &vk_physical_device));

		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceProperties(vk_physical_device, &vk_physical_device_properties);
		unsigned int queueFamilyPropertyCount = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queueFamilyPropertyCount, nullptr);
		queueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queueFamilyPropertyCount, queueFamilyProperties.data());

		float queuePriorities[1] = { 0.0 };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = 0;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = queuePriorities;

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		queueInfos.push_back(queueInfo);

		unsigned int extensionCount;

		vkGetPhysicalDeviceFeatures(vk_physical_device, &vk_physical_device_features);

		vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &memProperties);

		std::vector<char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.queueCreateInfoCount = queueInfos.size();
		deviceInfo.enabledExtensionCount = deviceExtensions.size();
		deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
		deviceInfo.pEnabledFeatures = &vk_physical_device_features;
		vk_chk_res(vkCreateDevice(vk_physical_device, &deviceInfo, nullptr, &vk_device));

		vkGetDeviceQueue(vk_device, 0, 0, &vk_graphics_queue);

		command_pool = new CommandPool;
		descriptorPool = new DescriptorPool;

		init_buffer();
		init_texture();
		init_material();
		init_shader();
		init_sampler();
		init_pick_up();
	}

	int Resolution::x() const
	{
		return _x;
	}

	int Resolution::y() const
	{
		return _y;
	}

	float Resolution::aspect() const
	{
		return _aspect;
	}

	long long Resolution::dirty_frame() const
	{
		return _dirty_frame;
	}

	void Resolution::set(int x, int y)
	{
		_x = x;
		_y = y;
		_aspect = (float)_x / _y;

		_dirty_frame = total_frame_count;
		add_after_frame_event([this]() {
			broadcast(this, MessageResolutionChange, false);
		});
	}

	void Resolution::set_x(int x)
	{
		set(x, _y);
	}

	void Resolution::set_y(int y)
	{
		set(_x, y);
	}

	Resolution resolution;
}
