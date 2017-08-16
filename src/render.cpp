#include <filesystem>
#include <regex>
#include <assert.h>
#include <sstream>
#include <stack>

#include "render.h"
#include "entity.h"
#include "core.h"

namespace tke
{
	VkFormat swapchainFormat;
	VkSampler plainSampler;
	VkSampler plainUnnormalizedSampler;
	VkSampler colorSampler;
	VkSampler colorBorderSampler;
	VkSampler colorWrapSampler;

	VkFence createFence()
	{
		VkFence fence;

		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		device.mtx.lock();
		auto res = vkCreateFence(device.v, &info, nullptr, &fence);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		return fence;
	}

	void destroyFence(VkFence fence)
	{
		device.mtx.lock();
		vkDestroyFence(device.v, fence, nullptr);
		device.mtx.unlock();
	}

	VkEvent createEvent()
	{
		VkEvent event;

		VkEventCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

		device.mtx.lock();
		auto res = vkCreateEvent(device.v, &info, nullptr, &event);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		return event;
	}

	void destroyEvent(VkEvent event)
	{
		device.mtx.lock();
		vkDestroyEvent(device.v, event, nullptr);
		device.mtx.unlock();
	}

	VkSemaphore createSemaphore()
	{
		VkSemaphore semaphore;

		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		device.mtx.lock();
		auto res = vkCreateSemaphore(device.v, &info, nullptr, &semaphore);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		return semaphore;
	}

	void destroySemaphore(VkSemaphore semaphore)
	{
		device.mtx.lock();
		vkDestroySemaphore(device.v, semaphore, nullptr);
		device.mtx.unlock();
	}

	void waitFence(VkFence fence)
	{
		device.mtx.lock();
		VkResult res;
		res = vkWaitForFences(device.v, 1, &fence, true, UINT64_MAX);
		assert(res == VK_SUCCESS);
		res = vkResetFences(device.v, 1, &fence);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	VkPipelineVertexInputStateCreateInfo vertexStateInfo(std::uint32_t bindingCount, VkVertexInputBindingDescription *pBindings, std::uint32_t attributeCount, VkVertexInputAttributeDescription *pAttributes)
	{
		VkPipelineVertexInputStateCreateInfo state = {};
		state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		state.vertexBindingDescriptionCount = bindingCount;
		state.pVertexBindingDescriptions = pBindings;
		state.vertexAttributeDescriptionCount = attributeCount;
		state.pVertexAttributeDescriptions = pAttributes;

		return state;
	}

	VkAttachmentDescription colorAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp)
	{
		VkAttachmentDescription desc = {};
		desc.format = format;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOp;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		return desc;
	}

	VkAttachmentDescription depthAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp)
	{
		VkAttachmentDescription desc = {};
		desc.format = format;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOp;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		return desc;
	}

	VkAttachmentDescription swapchainAttachmentDesc(VkAttachmentLoadOp loadOp)
	{
		VkAttachmentDescription desc = {};
		desc.format = swapchainFormat;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOp;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return desc;
	}

	VkSubpassDescription subpassDesc(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth, int inputCount, VkAttachmentReference *pInputs)
	{
		VkSubpassDescription desc = {};
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.inputAttachmentCount = inputCount;
		desc.pInputAttachments = pInputs;
		desc.colorAttachmentCount = colorCount;
		desc.pColorAttachments = pColors;
		desc.pDepthStencilAttachment = pDepth;
		return desc;
	}

	VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass)
	{
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = srcSubpass;
		dependency.dstSubpass = dstSubpass;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		return dependency;
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

		if (messageCode == 52) return VK_FALSE; // At Draw time the active render pass is incompatible w/ gfx pipeline

												  // ignore above

		if (messageCode == 101) return VK_FALSE; // vkQueuePresentKHR: Presenting image without calling vkGetPhysicalDeviceSurfaceSupportKHR
		if (messageCode == 100) return VK_FALSE; // vkCreateSwapChainKHR(): surface capabilities not retrieved for this physical device
		if (messageCode == 1922 || messageCode == 341838316) return VK_FALSE; // vkCreateSwapChainKHR(): pCreateInfo->surface is not known at this time to be supported for presentation by this device. The vkGetPhysicalDeviceSurfaceSupportKHR() must be called beforehand, and it must return VK_TRUE support with this surface for at least one queue family of this device
		if (messageCode == 24) return VK_FALSE; // Vertex buffers are bound to command buffer but no vertex buffers are attached to this Pipeline State Object.
		if (messageCode == 59) return VK_FALSE; // Descriptor set encountered the following validation error at vkCmdDrawIndexed() time: Descriptor is being used in draw but has not been updated.
		if (messageCode == 63) return VK_FALSE; // vkBeginCommandBuffer(): Secondary Command Buffers may perform better if a valid framebuffer parameter is specified.
		if (messageCode == 14) return VK_FALSE;
		if (messageCode == 12) return VK_FALSE; // Push constant range covering variable starting at offset not accessible from stage
		if (messageCode == 4) return VK_TRUE; //Pipeline needs renderpass information

		return VK_FALSE;
	}

	Err initRender(bool debug)
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
		if (res != VkResult::VK_SUCCESS) { return Err::eContextLost; }

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
		if (res != VkResult::VK_SUCCESS) { return Err::eContextLost; }

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

		swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

		// sampler must be created correctly

		{
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 0;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			res = vkCreateSampler(device.v, &samplerInfo, nullptr, &plainSampler);
			assert(res == VK_SUCCESS);
		}

		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_NEAREST;
			info.minFilter = VK_FILTER_NEAREST;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.anisotropyEnable = VK_FALSE;
			info.maxAnisotropy = 0;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			info.unnormalizedCoordinates = VK_TRUE;
			info.compareEnable = VK_FALSE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.mipLodBias = 0.0f;
			info.minLod = 0.0f;
			info.maxLod = 0.0f;

			res = vkCreateSampler(device.v, &info, nullptr, &plainUnnormalizedSampler);
			assert(res == VK_SUCCESS);
		}

		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.anisotropyEnable = VK_TRUE;
			info.maxAnisotropy = 16;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			info.unnormalizedCoordinates = VK_FALSE;
			info.compareEnable = VK_FALSE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.mipLodBias = 0.0f;
			info.minLod = 0.f;
			info.maxLod = 128.f;

			res = vkCreateSampler(device.v, &info, nullptr, &colorSampler);
			assert(res == VK_SUCCESS);
		}

		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			info.anisotropyEnable = VK_TRUE;
			info.maxAnisotropy = 16;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			info.unnormalizedCoordinates = VK_FALSE;
			info.compareEnable = VK_FALSE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.mipLodBias = 0.0f;
			info.minLod = 0.f;
			info.maxLod = 128.f;

			res = vkCreateSampler(device.v, &info, nullptr, &colorBorderSampler);
			assert(res == VK_SUCCESS);
		}

		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			info.anisotropyEnable = VK_TRUE;
			info.maxAnisotropy = 16;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			info.unnormalizedCoordinates = VK_FALSE;
			info.compareEnable = VK_FALSE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.mipLodBias = 0.0f;
			info.minLod = 0.f;
			info.maxLod = 128.f;

			res = vkCreateSampler(device.v, &info, nullptr, &colorWrapSampler);
			assert(res == VK_SUCCESS);
		}

		return Err::eNoErr;
	}

	static VkBlendFactor _vkBlendFactor(BlendFactor f) 
	{
		switch (f)
		{
		case BlendFactor::zero:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::one:
			return VK_BLEND_FACTOR_ONE;
		case BlendFactor::src_alpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::one_minus_src_alpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}
	};

	static VkDescriptorType _vkDescriptorType(DescriptorType t)
	{
		switch (t)
		{
		case DescriptorType::uniform_buffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::image_n_sampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
	}

	void ResourceBank::setBuffer(Buffer *p, const std::string &str)
	{
		bufferResources[str] = p;
	}

	void ResourceBank::setImage(Image *p, const std::string &str)
	{
		imageResources[str] = p;
	}

	Buffer *ResourceBank::getBuffer(const std::string &str)
	{
		auto it = bufferResources.find(str);
		if (it == bufferResources.end())
		{
			if (parents)
				return parents->getBuffer(str);
			else
				return nullptr;
		}
		return it->second;
	}

	Image *ResourceBank::getImage(const std::string &str)
	{
		auto it = imageResources.find(str);
		if (it == imageResources.end())
		{
			if (parents)
				return parents->getImage(str);
			else
				return nullptr;
		}
		return it->second;
	}

	ResourceBank::ResourceBank(ResourceBank *_parent)
	{
		parents = _parent;
	}

	ResourceBank globalResource(nullptr);

	ShaderModule::~ShaderModule()
	{
		device.mtx.lock();
		vkDestroyShaderModule(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<ShaderModule*> shaderModules;

	Stage::Stage(Pipeline *_parent)
	{
		parents = _parent;
	}

	static bool _findDefine(const std::vector<std::string> &vec, const std::string &def, bool b)
	{
		if (b)
		{
			for (auto &v : vec)
			{
				if (v == def)
					return true;
			}
			return false;
		}
		else
		{
			for (auto &v : vec)
			{
				if (v == def)
					return false;
			}
			return true;
		}
	}

	static std::string _last_compiled_stage_text;
	static std::vector<int> _currentUboBinding;
	void Stage::create()
	{
		std::vector<std::string> defines;
		for (auto &m : parents->shaderMacros)
		{
			if (((int)m.stage & (int)type))
				defines.push_back(m.value);
		}

		// format the shader path, so that they can reuse if they refer the same one
		auto stageFilename = std::experimental::filesystem::canonical(parents->filepath + "/" + filename).string();
		for (auto m : shaderModules)
		{
			if (m->filename == stageFilename)
			{
				if (defines.size() != m->defines.size()) continue;

				bool same = true;
				for (int i = 0; i < defines.size(); i++)
				{
					if (defines[i] != m->defines[i])
					{
						same = false;
						break;
					}
				}

				if (same)
				{
					m->refCount++;
					module = m;
					return;
				}
			}
		}

		module = new ShaderModule;
		module->filename = stageFilename;
		module->defines.insert(module->defines.begin(), defines.begin(), defines.end());
		shaderModules.push_back(module);

		// Warnning:push constants in different stages must be merged, or else they would not reflect properly.

		{
			auto file_path = std::experimental::filesystem::path(stageFilename).parent_path().string();
			tke::OnceFileBuffer file(stageFilename);

			std::stringstream ss(file.data);

			int lineNum = 0;
			std::string stageText = "";
			stageText += "#version 450 core\n"; lineNum++;
			stageText += "#extension GL_ARB_separate_shader_objects : enable\n"; lineNum++;
			stageText += "#extension GL_ARB_shading_language_420pack : enable\n\n"; lineNum++;
			for (auto &m : defines)
			{
				stageText += "#define " + m + "\n";
				lineNum++;
			}
			int fullLineNum = lineNum;

			std::stack<std::pair<bool, bool>> states; // first current accept, second this con accepted
			states.push({ true, false });

			std::vector<std::tuple<int, int, int>> includeFileDatas;

			std::string line;
			while (!ss.eof())
			{
				std::getline(ss, line);

				std::regex pattern;
				std::smatch match;

				if (std::regex_search(line, match, pattern = R"(#(el)?if\s+(\!)?defined\(([\w_]*)\)[\s&]*)"))
				{
					bool isElse = match[1].matched;
					bool ok;
					if ((isElse && !states.top().second) || (!isElse && states.top().first))
					{
						std::vector<std::pair<std::string, bool>> cons;
						if (match[2].matched)
							cons.emplace_back(match[3].str(), false);
						else
							cons.emplace_back(match[3].str(), true);
						std::string str = match.suffix();
						while (std::regex_search(str, match, pattern = R"((\!)?defined\(([\w_]*)\)[\s&]*)"))
						{
							if (match[1].matched)
								cons.emplace_back(match[2].str(), false);
							else
								cons.emplace_back(match[2].str(), true);
							str = match.suffix();
						}

						ok = true;
						for (auto &c : cons)
						{
							if (!_findDefine(defines, c.first, c.second))
							{
								ok = false;
								break;
							}
						}
					}

					if (isElse)
					{
						if (states.top().second)
						{
							states.top().first = false;
						}
						else
						{
							states.top().first = ok;
							states.top().second = ok;
						}
					}
					else
					{
						if (states.top().first)
							states.push({ ok, ok });
						else
							states.push({ false, true });
					}
				}
				else if (std::regex_search(line, match, pattern = R"(#else)"))
				{
					states.top().first = !states.top().first;
					states.top().second = true;
				}
				else if (std::regex_search(line, match, pattern = R"(#endif)"))
				{
					states.pop();
				}
				else if (states.top().first && std::regex_search(line, match, pattern = R"(#define\s+([\w_]*))"))
				{
					defines.push_back(match[1].str());

					stageText += line + "\n";

					fullLineNum += 1;
					lineNum++;
				}
				else if (states.top().first && std::regex_search(line, match, pattern = R"(#include\s+\"([\w\.\\]*)\")"))
				{
					tke::OnceFileBuffer includeFile(file_path + "/" + match[1].str());
					stageText += includeFile.data;
					stageText += "\n";

					auto includeFileLineNum = tke::lineNumber(includeFile.data);
					includeFileDatas.emplace_back(lineNum, fullLineNum, includeFileLineNum);

					fullLineNum += includeFileLineNum;
					lineNum++;
				}
				else if (states.top().first && std::regex_search(line, match, pattern = R"(layout\s*\((\s*set\s*=\s*([0-9]+)\s*,\s*)?\s*binding\s*=\s*((TKE_UBO_BINDING)|([0-9]+))\s*\)\s*uniform\s+((sampler2D)\s+)?([\w_]+)\s*(\[\s*([0-9]+)\s*\])?)"))
				{
					auto set = match[2].matched ? std::stoi(match[2].str()) : 0;
					if (set >= _currentUboBinding.size())
						_currentUboBinding.resize(set + 1);
					if (set >= module->descriptors.size())
						module->descriptors.resize(set + 1);

					Descriptor d;
					d.name = match[8].str();
					d.binding = -1;
					for (int i = 0; i < 5; i++)
					{
						if (parents->stages[i])
						{
							auto s = parents->stages[i];
							if (s->type != type && s->module && s->module->descriptors.size() > set)
							{
								for (auto &_d : s->module->descriptors[set])
								{
									if (_d.name == d.name)
									{
										d.binding = _d.binding;
										d.type = _d.type;
										d.count = _d.count;
										break;
									}
								}
								if (d.binding != -1)
									break;
							}
						}
					}

					if (d.binding == -1)
					{
						d.binding = _currentUboBinding[set];
						d.type = match[7].matched ? DescriptorType::image_n_sampler : DescriptorType::uniform_buffer;
						d.count = match[10].matched ? std::stoi(match[10].str()) : 1;
						module->descriptors[set].push_back(d);
						_currentUboBinding[set]++;
					}
					else
					{
						module->descriptors[set].push_back(d);
					}
					line = std::regex_replace(line, pattern = R"(TKE_UBO_BINDING)", std::to_string(d.binding));

					stageText += line + "\n";

					fullLineNum += 1;
					lineNum++;
				}
				else if (states.top().first)
				{
					stageText += line + "\n";

					fullLineNum += 1;
					lineNum++;
				}
			}

			{
				_last_compiled_stage_text = stageText;
				std::ofstream file("temp.glsl");
				file.write(stageText.c_str(), stageText.size());
				file.close();
			}

			tke::exec("cmd", std::string("/C glslangValidator ") + enginePath + "src/my_glslValidator_config.conf -V temp.glsl -S " + tke::StageNameByType(type) + " -q -o temp.spv > output.txt");

			bool error = false;

			std::string output;
			{
				tke::OnceFileBuffer outputFile("output.txt");
				output = outputFile.data;
				output += "\n";

				// analyzing the reflection

				enum ReflectionType
				{
					eNull = -1,
					eUniform = 0,
					eUniformBlock = 1,
					eVertexAttribute = 2
				};

				struct Reflection
				{
					int COUNT = 1; // special for UBO

					ReflectionType reflectionType;
					std::string name;
					int offset;
					std::string type;
					int size;
					int index;
					int binding;
				};

				struct ReflectionManager
				{
					std::vector<Reflection> rs;
					void add(Reflection &_r)
					{
						if (_r.reflectionType == eUniformBlock && _r.binding != -1)
						{
							for (auto &r : rs)
							{
								if (r.binding == _r.binding)
								{
									r.COUNT++;
									return;
								}
							}
						}
						rs.push_back(_r);
					}
				};

				ReflectionManager reflections;
				Reflection currentReflection;

				ReflectionType currentReflectionType = eNull;

				int error_count = 0;

				std::stringstream ss(outputFile.data);
				std::string line;
				while (!ss.eof())
				{
					std::getline(ss, line);

					std::regex pattern;
					std::smatch match;
					if (std::regex_search(line, match, pattern = R"(glslangValidator: Error unable to open input file)"))
					{
						error = true;
					}
					else if (std::regex_search(line, match, pattern = R"(ERROR: temp.glsl:([-0-9][-a-zA-Z0-9]*))"))
					{
						error = true;

						auto n = std::stoi(match[1].str());

						for (auto it = includeFileDatas.rbegin(); it != includeFileDatas.rend(); it++)
						{
							if (n > std::get<1>(*it) + std::get<2>(*it))
							{
								n = n - std::get<1>(*it) + std::get<0>(*it) - std::get<2>(*it) - 1;
								break;
							}
							else if (n > std::get<1>(*it))
							{
								n = std::get<0>(*it);
								break;
							}
						}

						error_count++;
						output += std::string("The ") + std::to_string(error_count) + "th Error, redirect line num:" + std::to_string(n) + "\n";
					}
					else if (std::regex_search(line, match, pattern = R"(Uniform reflection:)"))
					{
						currentReflectionType = eUniform;
					}
					else if (std::regex_search(line, match, pattern = R"(Uniform block reflection:)"))
					{
						currentReflectionType = eUniformBlock;
					}
					else if (std::regex_search(line, match, pattern = R"(Vertex attribute reflection:)"))
					{
						currentReflectionType = eVertexAttribute;
					}
					else if (std::regex_search(line, match, pattern = R"(([_a-zA-Z][_a-zA-Z0-9.]*)[\[\]0-9]*: offset ([-0-9][-a-zA-Z0-9]*), type ([_a-zA-Z0-9.]*), size ([-0-9][-a-zA-Z0-9]*), index ([-0-9][-a-zA-Z0-9]*), binding ([-0-9][-a-zA-Z0-9]*))"))
					{
						Reflection reflection;
						reflection.reflectionType = currentReflectionType;
						reflection.name = match[1].str();
						reflection.offset = std::stoi(match[2].str());
						reflection.type = match[3].str();
						reflection.size = std::stoi(match[4].str());
						reflection.index = std::stoi(match[5].str());
						reflection.binding = std::stoi(match[6].str());
						reflections.add(reflection);
					}
				}

				for (auto &r : reflections.rs)
				{
					switch (r.reflectionType)
					{
					case eUniform:
						if (r.binding != -1 && r.type == "8b5e") // SAMPLER
						{
							// processed above
						}
						break;
					case eUniformBlock:
						if (r.binding != -1) // UBO
						{
							// processed above
						}
						else // PC
						{
							tke::PushConstantRange p;
							p.offset = 0; // 0 always
							p.size = r.size;
							module->pushConstantRanges.push_back(p);
						}
						break;
					}
				}
			}

			if (!error)
			{
				OnceFileBuffer file("temp.spv");
				VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.codeSize = file.length;
				shaderModuleCreateInfo.pCode = (uint32_t*)file.data;

				device.mtx.lock();
				auto res = vkCreateShaderModule(device.v, &shaderModuleCreateInfo, nullptr, &module->v);
				assert(res == VK_SUCCESS);
				device.mtx.unlock();

				DeleteFileA("temp.spv");
			}
			else
			{
				assert(false);
				MessageBox(NULL, output.c_str(), stageFilename.c_str(), 0);
				exit(1);
			}

			DeleteFileA("output.txt");
			DeleteFileA("temp.glsl");
		}
	}

	Stage::~Stage()
	{
		module->refCount--;
		if (module->refCount == 0)
		{
			for (auto it = shaderModules.begin(); it != shaderModules.end(); it++)
			{
				if (*it == module)
				{
					shaderModules.erase(it);
					delete module;
					return;
				}
			}
		}
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		device.mtx.lock();
		vkDestroyDescriptorSetLayout(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<DescriptorSetLayout*> _descriptorSetLayouts;

	PipelineLayout::~PipelineLayout()
	{
		device.mtx.lock();
		vkDestroyPipelineLayout(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<PipelineLayout*> pipelineLayouts;
}
