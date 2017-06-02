#include <experimental/filesystem>
#include <regex>
#include <assert.h>

#include "render.h"
#include "core.h"
#include "gui.h"
#include "model.h"
#include "image.file.h"

namespace tke
{
	void Device::waitIdle()
	{
		cs.lock();
		vkDeviceWaitIdle(v);
		cs.unlock();
	}

	void Queue::waitIdle()
	{
		cs.lock();
		vkQueueWaitIdle(v);
		cs.unlock();
	}

	void Queue::submit(VkSemaphore waitSemaphore, VkCommandBuffer cmd, VkSemaphore signalSemaphore)
	{
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphore;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &cmd;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &signalSemaphore;

		graphicsQueue.cs.lock();
		auto res = vkQueueSubmit(graphicsQueue.v, 1, &info, VK_NULL_HANDLE);
		assert(res == VK_SUCCESS);
		graphicsQueue.cs.unlock();
	}

	void Queue::submit(VkSemaphore waitSemaphore, int count, VkCommandBuffer *cmds, VkSemaphore signalSemaphore)
	{
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphore;
		info.commandBufferCount = count;
		info.pCommandBuffers = cmds;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &signalSemaphore;

		graphicsQueue.cs.lock();
		auto res = vkQueueSubmit(graphicsQueue.v, 1, &info, VK_NULL_HANDLE);
		assert(res == VK_SUCCESS);
		graphicsQueue.cs.unlock();
	}

	void Queue::submitFence(VkSemaphore waitSemaphore, int count, VkCommandBuffer *cmds, VkFence fence)
	{
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphore;
		info.commandBufferCount = count;
		info.pCommandBuffers = cmds;

		graphicsQueue.cs.lock();
		auto res = vkQueueSubmit(graphicsQueue.v, 1, &info, fence);
		assert(res == VK_SUCCESS);
		graphicsQueue.cs.unlock();
	}

	Instance inst;
	Device device;
	Queue graphicsQueue;

	void CommandPool::create()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cmdPoolInfo.queueFamilyIndex = 0;
		device.cs.lock();
		auto res = vkCreateCommandPool(device.v, &cmdPoolInfo, nullptr, &pool);
		device.cs.unlock();
		assert(res == VK_SUCCESS);
	}

	void CommandPool::destroy()
	{
		device.cs.lock();
		vkDestroyCommandPool(device.v, pool, nullptr);
		device.cs.unlock();
	}

	VkCommandBuffer CommandPool::allocate()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer cmd;
		device.cs.lock();
		auto res = vkAllocateCommandBuffers(device.v, &allocInfo, &cmd);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return cmd;
	}

	VkCommandBuffer CommandPool::allocateSecondary()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandPool = pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer cmd;
		device.cs.lock();
		auto res = vkAllocateCommandBuffers(device.v, &allocInfo, &cmd);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return cmd;
	}

	void CommandPool::free(VkCommandBuffer cmd)
	{
		device.cs.lock();
		vkFreeCommandBuffers(device.v, pool, 1, &cmd);
		device.cs.unlock();
	}

	VkCommandBuffer CommandPool::begineOnce()
	{
		auto cmd = allocate();
		beginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return cmd;
	}

	void CommandPool::endOnce(VkCommandBuffer cmd)
	{
		VkResult res;

		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		device.cs.lock();
		graphicsQueue.cs.lock();
		res = vkQueueSubmit(graphicsQueue.v, 1, &submitInfo, VK_NULL_HANDLE);
		assert(res == VK_SUCCESS);
		res = vkQueueWaitIdle(graphicsQueue.v);
		assert(res == VK_SUCCESS);
		graphicsQueue.cs.unlock();
		device.cs.unlock();

		free(cmd);
	}

	void CommandPool::cmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset, size_t dstOffset)
	{
		auto commandBuffer = begineOnce();

		VkBufferCopy region = {};
		region.size = size;
		region.srcOffset = srcOffset;
		region.dstOffset = dstOffset;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);

		endOnce(commandBuffer);
	}

	void CommandPool::cmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges)
	{
		auto commandBuffer = begineOnce();

		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, count, ranges);

		endOnce(commandBuffer);
	}

	void CommandPool::cmdUpdateBuffer(void *data, size_t size, StagingBuffer &stagingBuffer, VkBuffer &Buffer)
	{
		void* map = stagingBuffer.map(0, size);
		memcpy(map, data, size);
		stagingBuffer.unmap();

		cmdCopyBuffer(stagingBuffer.m_buffer, Buffer, size);
	}

	void CommandPool::cmdCopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
	{
		auto commandBuffer = begineOnce();

		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;

		VkImageCopy region = {};
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;

		vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endOnce(commandBuffer);
	}

	CommandPool commandPool;

	void DescriptrPool::create()
	{
		VkDescriptorPoolSize descriptorPoolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000 },
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
		descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
		descriptorPoolInfo.maxSets = 256;
		device.cs.lock();
		auto res = vkCreateDescriptorPool(device.v, &descriptorPoolInfo, nullptr, &pool);
		device.cs.unlock();
		assert(res == VK_SUCCESS);
	}

	VkDescriptorSet DescriptrPool::allocate(VkDescriptorSetLayout *pLayout)
	{
		VkDescriptorSetAllocateInfo descriptorSetInfo = {};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = pool;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = pLayout;

		VkDescriptorSet descriptorSet;
		device.cs.lock();
		auto res = vkAllocateDescriptorSets(device.v, &descriptorSetInfo, &descriptorSet);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return descriptorSet;
	}

	void DescriptrPool::free(VkDescriptorSet set)
	{
		device.cs.lock();
		auto res = vkFreeDescriptorSets(device.v, pool, 1, &set);
		assert(res == VK_SUCCESS);
		device.cs.unlock();
	}

	void DescriptrPool::addWrite(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo *pImageInfo, uint32_t dstArrayElement)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = descriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = dstArrayElement;
		write.descriptorType = type;
		write.descriptorCount = 1;
		write.pImageInfo = pImageInfo;

		writes.push_back(write);
	}

	void DescriptrPool::addWrite(VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo *pBufferInfo, uint32_t dstArrayElement)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = descriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = dstArrayElement;
		write.descriptorType = type;
		write.descriptorCount = 1;
		write.pBufferInfo = pBufferInfo;

		writes.push_back(write);
	}

	void DescriptrPool::update()
	{
		if (writes.size() == 0) 
			return;

		device.cs.lock();
		vkUpdateDescriptorSets(device.v, writes.size(), writes.data(), 0, nullptr);
		writes.clear();
		device.cs.unlock();
	}

	DescriptrPool descriptorPool;

	std::vector<Framebuffer> framebuffers;
	VkFramebuffer createFramebuffer(int cx, int cy, VkRenderPass renderPass, std::vector<VkImageView> &views)
	{
		for (auto &f : framebuffers)
		{
			if (f.views.size() == views.size())
			{
				bool same = true;
				for (auto i = 0; i < f.views.size(); i++)
				{
					if (f.views[i] != views[i])
					{
						same = false;
						break;
					}
				}
				if (same)
					return f.v;
			}
		}
		Framebuffer framebuffer;
		framebuffer.views.insert(framebuffer.views.begin(), views.begin(), views.end());

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = cx;
		info.height = cy;
		info.layers = 1;
		info.renderPass = renderPass;
		info.attachmentCount = views.size();
		info.pAttachments = views.data();

		device.cs.lock();
		auto res = vkCreateFramebuffer(device.v, &info, nullptr, &framebuffer.v);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		framebuffers.push_back(framebuffer);
		return framebuffer.v;
	}

	void destroyFramebuffer(VkFramebuffer v)
	{
		for (auto it = framebuffers.begin(); it != framebuffers.end(); it++)
		{
			if (it->v == v)
			{
				framebuffers.erase(it);

				device.cs.lock();
				vkDestroyFramebuffer(device.v, v, nullptr);
				device.cs.unlock();

				return;
			}
		}
	}

	VkFormat swapchainFormat;
	VkSampler plainSampler;
	VkSampler plainUnnormalizedSampler;
	VkSampler colorSampler;
	VkSampler colorBorderSampler;

	void beginCommandBuffer(VkCommandBuffer cmd, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo *pInheritance)
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = flags;
		info.pInheritanceInfo = pInheritance;
		auto res = vkBeginCommandBuffer(cmd, &info);
		assert(res == VK_SUCCESS);
	}

	VkFence createFence()
	{
		VkFence fence;

		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		device.cs.lock();
		auto res = vkCreateFence(device.v, &info, nullptr, &fence);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return fence;
	}

	void destroyFence(VkFence fence)
	{
		device.cs.lock();
		vkDestroyFence(device.v, fence, nullptr);
		device.cs.unlock();
	}

	VkEvent createEvent()
	{
		VkEvent event;

		VkEventCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

		device.cs.lock();
		auto res = vkCreateEvent(device.v, &info, nullptr, &event);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return event;
	}

	void destroyEvent(VkEvent event)
	{
		device.cs.lock();
		vkDestroyEvent(device.v, event, nullptr);
		device.cs.unlock();
	}

	VkSemaphore createSemaphore()
	{
		VkSemaphore semaphore;

		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		device.cs.lock();
		auto res = vkCreateSemaphore(device.v, &info, nullptr, &semaphore);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return semaphore;
	}

	void destroySemaphore(VkSemaphore semaphore)
	{
		device.cs.lock();
		vkDestroySemaphore(device.v, semaphore, nullptr);
		device.cs.unlock();
	}

	void waitFence(VkFence fence)
	{
		device.cs.lock();
		VkResult res;
		res = vkWaitForFences(device.v, 1, &fence, true, UINT64_MAX);
		assert(res == VK_SUCCESS);
		res = vkResetFences(device.v, 1, &fence);
		assert(res == VK_SUCCESS);
		device.cs.unlock();
	}

	static VkPhysicalDeviceMemoryProperties memProperties;

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

	VkRenderPass createRenderPass(std::uint32_t attachmentCount, VkAttachmentDescription *pAttachments, std::uint32_t subpassCount, VkSubpassDescription *pSubpasses, std::uint32_t dependencyCount, VkSubpassDependency *pDependencies)
	{
		VkRenderPass renderPass;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = attachmentCount;
		info.pAttachments = pAttachments;
		info.subpassCount = subpassCount;
		info.pSubpasses = pSubpasses;
		info.dependencyCount = dependencyCount;
		info.pDependencies = pDependencies;

		device.cs.lock();
		auto res = vkCreateRenderPass(device.v, &info, nullptr, &renderPass);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		return renderPass;
	}

	void destroyRenderPass(VkRenderPass rp)
	{
		device.cs.lock();
		vkDestroyRenderPass(device.v, rp, nullptr);
		device.cs.unlock();
	}

	VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer framebuffer, std::uint32_t cx, std::uint32_t cy, std::uint32_t clearValueCount, VkClearValue *pClearValues)
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = renderPass;
		info.framebuffer = framebuffer;
		info.renderArea.extent.width = cx;
		info.renderArea.extent.height = cy;
		info.clearValueCount = clearValueCount;
		info.pClearValues = pClearValues;

		return info;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		if (messageCode == 1) return VK_FALSE; // THREADING ERROR, 0.0 what is this
		if (messageCode == 2) return VK_FALSE; // Vertex attribute not consumed by vertex shader, never mind
		if (messageCode == 5) return VK_FALSE; // SPIR-V module not valid: Operand 4 of MemberDecorate requires one of these capabilities: MultiViewport 
		if (messageCode == 6) return VK_FALSE; // Image layout should be attachment optimal but got general, never mind
		if (messageCode == 8) return VK_FALSE; // Your fucking computer is not support anisotropy, never mind
		if (messageCode == 13) return VK_FALSE; // Shader expects at least n descriptors but only less provided, never mind
		if (messageCode == 15) return VK_FALSE; // Shader requires VkPhysicalDeviceFeatures::tessellationShader but is not enabled on the device, never mind
		if (messageCode == 53) return VK_FALSE; // You have gave more clear values, never mind
		if (messageCode == 54) return VK_FALSE; // vkCreateDevice: pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex (= 0) is not less than any previously obtained pQueueFamilyPropertyCount from vkGetPhysicalDeviceQueueFamilyProperties (the pQueueFamilyPropertyCount was never obtained)
		if (messageCode == 100) return VK_FALSE; // vkCreateSwapChainKHR(): surface capabilities not retrieved for this physical device
		if (messageCode == 101) return VK_FALSE; // vkQueuePresentKHR: Presenting image without calling vkGetPhysicalDeviceSurfaceSupportKHR
		if (messageCode == 1922) return VK_FALSE; // vkCreateSwapChainKHR(): pCreateInfo->surface is not known at this time to be supported for presentation by this device. The vkGetPhysicalDeviceSurfaceSupportKHR() must be called beforehand, and it must return VK_TRUE support with this surface for at least one queue family of this device

												  // ignore above

		if (messageCode == 61) return VK_FALSE; // Some descriptor maybe used before any update, never mind
		if (messageCode == 24) return VK_FALSE; // Vertex buffers are bound to command buffer but no vertex buffers are attached to this Pipeline State Object.
		if (messageCode == 59) return VK_FALSE; // Descriptor set encountered the following validation error at vkCmdDrawIndexed() time: Descriptor is being used in draw but has not been updated.
		if (messageCode == 63) return VK_FALSE; // vkBeginCommandBuffer(): Secondary Command Buffers may perform better if a valid framebuffer parameter is specified.
		if (messageCode == 14) return VK_FALSE;
		if (messageCode == 12) return VK_FALSE; // Push constant range covering variable starting at offset not accessible from stage
		return VK_FALSE;
	}

	Err initRender(const char *appName, bool debug)
	{
		VkResult res;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
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
			callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			callbackCreateInfo.pfnCallback = &_vkDebugCallback;
			callbackCreateInfo.pUserData = nullptr;

			VkDebugReportCallbackEXT callback;
			res = _vkCreateDebugReportCallbackEXT(inst.v, &callbackCreateInfo, nullptr, &callback);
			assert(res == VK_SUCCESS);
		}

		VkPhysicalDevice physicalDevice;
		uint32_t gpuCount = 1;
		res = vkEnumeratePhysicalDevices(inst.v, &gpuCount, &physicalDevice);
		if (res != VkResult::VK_SUCCESS) { return Err::eContextLost; }

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

		commandPool.create();

		descriptorPool.create();

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
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
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

		return Err::eNoErr;
	}

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		return -1;
	}

	static void buffer_create(Buffer *p)
	{
		VkResult res;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = p->m_size;
		bufferInfo.usage = p->usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		device.cs.lock();

		res = vkCreateBuffer(device.v, &bufferInfo, nullptr, &p->m_buffer);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.v, p->m_buffer, &memRequirements);

		assert(p->m_size <= memRequirements.size);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, p->memoryProperty);

		res = vkAllocateMemory(device.v, &allocInfo, nullptr, &p->m_memory);
		assert(res == VK_SUCCESS);

		res = vkBindBufferMemory(device.v, p->m_buffer, p->m_memory, 0);
		assert(res == VK_SUCCESS);

		device.cs.unlock();
	}

	static void buffer_destroy(Buffer *p)
	{
		device.cs.lock();
		vkFreeMemory(device.v, p->m_memory, nullptr);
		vkDestroyBuffer(device.v, p->m_buffer, nullptr);
		device.cs.unlock();
	}

	Buffer::Buffer(size_t size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty)
	{
		m_size = size;
		usage = _usage;
		memoryProperty = _memoryProperty;

		buffer_create(this);
	}

	Buffer::~Buffer()
	{
		buffer_destroy(this);
	}

	void Buffer::recreate(size_t size)
	{
		buffer_destroy(this);
		m_size = size;
		buffer_create(this);
	}

	StagingBuffer::StagingBuffer(size_t size)
		:Buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
	}

	void *StagingBuffer::map(size_t offset, size_t size)
	{
		void *map;
		device.cs.lock();
		auto res = vkMapMemory(device.v, m_memory, offset, size, 0, &map);
		assert(res == VK_SUCCESS);
		device.cs.unlock();
		return map;
	}

	void StagingBuffer::unmap()
	{
		device.cs.lock();
		vkUnmapMemory(device.v, m_memory);
		device.cs.unlock();
	}

	static void buffer_copy(NonStagingBufferAbstract *p, void *data)
	{
		StagingBuffer stagingBuffer(p->m_size);

		void* map = stagingBuffer.map(0, p->m_size);
		memcpy(map, data, p->m_size);
		stagingBuffer.unmap();
		commandPool.cmdCopyBuffer(stagingBuffer.m_buffer, p->m_buffer, p->m_size);
	}

	NonStagingBufferAbstract::NonStagingBufferAbstract(size_t size, VkBufferUsageFlags usage, void *data)
		:Buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		if (data)
			buffer_copy(this, data);
	}

	void NonStagingBufferAbstract::recreate(size_t size, void *data)
	{
		Buffer::recreate(size);
		if (data)
			buffer_copy(this, data);
	}

	void NonStagingBufferAbstract::update(void *data, StagingBuffer &stagingBuffer, size_t size)
	{
		if (size == 0) size = m_size;
		commandPool.cmdUpdateBuffer(data, size, stagingBuffer, m_buffer);
	}

	ShaderManipulatableBufferAbstract::ShaderManipulatableBufferAbstract(size_t size, VkBufferUsageFlags usage)
		:NonStagingBufferAbstract(size, usage)
	{
		m_info = {};
		m_info.buffer = m_buffer;
		m_info.range = m_size;
	}

	UniformBuffer::UniformBuffer(size_t size)
		:ShaderManipulatableBufferAbstract(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
	}

	VertexBuffer::VertexBuffer(size_t size, void *data)
		: NonStagingBufferAbstract(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data)
	{
	}

	void VertexBuffer::bind(VkCommandBuffer cmd)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &m_buffer, offsets);
	}

	IndexBuffer::IndexBuffer(size_t size, void *data)
		:NonStagingBufferAbstract(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, data)
	{
	}

	void IndexBuffer::bind(VkCommandBuffer cmd)
	{
		vkCmdBindIndexBuffer(cmd, m_buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	IndirectVertexBuffer::IndirectVertexBuffer(size_t size)
		:NonStagingBufferAbstract(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
	{
	}

	IndirectIndexBuffer::IndirectIndexBuffer(size_t size)
		: NonStagingBufferAbstract(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
	{
	}

	void Image::transitionLayout(int level, VkImageAspectFlags aspect, VkImageLayout layout)
	{
		auto commandBuffer = commandPool.begineOnce();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = m_layout;
		barrier.newLayout = layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_image;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = level;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (m_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		else if (m_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		else if (m_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		else if (m_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		else if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		else if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		commandPool.endOnce(commandBuffer);

		m_layout = layout;
	}

	void Image::fillData(int level, void *data, size_t size, VkImageAspectFlags aspect)
	{
		transitionLayout(level, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		StagingBuffer stagingBuffer(size);

		void* map = stagingBuffer.map(0, size);
		memcpy(map, data, size);
		stagingBuffer.unmap();

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = getWidth(level);
		region.imageExtent.height = getHeight(level);
		region.imageExtent.depth = 1;
		region.bufferOffset = 0;

		auto cmd = commandPool.begineOnce();
		vkCmdCopyBufferToImage(cmd, stagingBuffer.m_buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		commandPool.endOnce(cmd);
	}

	Image::Image(int w, int h, VkFormat format, VkImageUsageFlags usage, int mipmapLevels, void *data, size_t size, VkImageAspectFlags aspect)
	{
		m_width = w;
		m_height = h;
		m_mipmapLevels = mipmapLevels;
		m_format = format;
		if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT)
			type = eDepth;
		else if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			type = eDepthStencil;
		else
			type = eColor;

		if (aspect == 0)
		{
			if (type == eColor || type == eSwapchain)
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			else
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		VkResult res;

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = w;
		imageInfo.extent.height = h;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = m_mipmapLevels;
		imageInfo.arrayLayers = m_arrayLayers;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		device.cs.lock();

		res = vkCreateImage(device.v, &imageInfo, nullptr, &m_image);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.v, m_image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(device.v, &allocInfo, nullptr, &m_memory);
		assert(res == VK_SUCCESS);

		res = vkBindImageMemory(device.v, m_image, m_memory, 0);
		assert(res == VK_SUCCESS);

		device.cs.unlock();

		m_size = memRequirements.size;

		if (data && size)
		{
			assert(size <= m_size);
			fillData(0, data, size, aspect);
		}
		transitionLayout(0, aspect, VK_IMAGE_LAYOUT_GENERAL);
	}

	Image::Image(Type _type, VkImage _image, int w, int h, VkFormat format)
	{
		type = _type;
		m_image = _image;
		m_width = w;
		m_height = h;
		m_format = format;
	}

	Image::~Image()
	{
		device.cs.lock();
		for (auto &v : views)
			vkDestroyImageView(device.v, v.view, nullptr);
		if (type != Type::eSwapchain)
		{
			vkFreeMemory(device.v, m_memory, nullptr);
			vkDestroyImage(device.v, m_image, nullptr);
		}
		device.cs.unlock();
	}

	VkImageView Image::getView(VkImageAspectFlags aspect, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		if (aspect == 0)
		{
			if (type == eColor || type == eSwapchain)
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			else
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		for (auto &view : views)
		{
			if (view.aspect == aspect && view.baseLevel == baseLevel && view.levelCount == levelCount &&
				view.baseLayer == baseLayer && view.layerCount == layerCount)
				return view.view;
		}

		View view;
		view.aspect = aspect;
		view.baseLevel = baseLevel;
		view.levelCount = levelCount;
		view.baseLayer = baseLayer;
		view.layerCount = layerCount;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = m_image;
		info.viewType = m_viewType;
		info.format = m_format;
		info.subresourceRange.aspectMask = aspect;
		info.subresourceRange.baseMipLevel = baseLevel;
		info.subresourceRange.levelCount = levelCount;
		info.subresourceRange.baseArrayLayer = baseLayer;
		info.subresourceRange.layerCount = layerCount;

		device.cs.lock();
		auto res = vkCreateImageView(device.v, &info, nullptr, &view.view);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		views.push_back(view);
		return view.view;
	}

	VkDescriptorImageInfo *Image::getInfo(VkSampler sampler, VkImageAspectFlags aspect, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		auto view = getView(aspect, baseLevel, levelCount, baseLayer, layerCount);
		for (auto &info : infos)
		{
			if (info.imageView == view && info.sampler == sampler)
				return &info;
		}
		VkDescriptorImageInfo info;
		info.imageView = view;
		info.sampler = sampler;
		info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		infos.push_back(info);
		return &infos.back();
	}

	int Image::getWidth(int mipmapLevel) const
	{
		int w = m_width;
		for (;;)
		{
			if (mipmapLevel <= 0 || w < 2)
				break;
			mipmapLevel--;
			w >>= 1;
		}
		return w;
	}

	int Image::getHeight(int mipmapLevel) const
	{
		int h = m_height;
		for (;;)
		{
			if (mipmapLevel <= 0 || h < 2)
				break;
			mipmapLevel--;
			h >>= 1;
		}
		return h;
	}

	unsigned char Image::getPixel(int x, int y, int off) const
	{
		//if (!m_data || x >= m_width || y >= m_height)
		//	return 0;

		//int pixelLength = 4;
		//int lineLength = PITCH(m_width);
		//lineLength = PITCH(m_width * pixelLength);

		//return m_data[x * pixelLength + y * lineLength + off];

		return 0;
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
		case DescriptorType::storage_buffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::storage_image:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::image_n_sampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::input_attachment:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
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

	void ResourceBank::setModel(Model *p, const std::string &str)
	{
		modelResources[str] = p;
	}

	void ResourceBank::setPipeline(Pipeline *p, const std::string &str)
	{
		pipelineResources[str] = p;
	}

	void ResourceBank::setCmd(VkCommandBuffer p, const std::string &str)
	{
		cmdResources[str] = p;
	}

	Buffer *ResourceBank::getBuffer(const std::string &str)
	{
		auto it = bufferResources.find(str);
		if (it == bufferResources.end())
		{
			if (parent)
				return parent->getBuffer(str);
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
			if (parent)
				return parent->getImage(str);
			else
				return nullptr;
		}
		return it->second;
	}

	Model *ResourceBank::getModel(const std::string &str)
	{
		auto it = modelResources.find(str);
		if (it == modelResources.end())
		{
			if (parent)
				return parent->getModel(str);
			else
				return nullptr;
		}
		return it->second;
	}

	Pipeline *ResourceBank::getPipeline(const std::string &str)
	{
		auto it = pipelineResources.find(str);
		if (it == pipelineResources.end())
		{
			if (parent)
				return parent->getPipeline(str);
			else
				return nullptr;
		}
		return it->second;
	}

	VkCommandBuffer ResourceBank::getCmd(const std::string &str)
	{
		auto it = cmdResources.find(str);
		if (it == cmdResources.end())
		{
			if (parent)
				return parent->getCmd(str);
			else
				return nullptr;
		}
		return it->second;
	}

	ResourceBank::ResourceBank(ResourceBank *_parent)
	{
		parent = _parent;
	}

	ResourceBank::~ResourceBank()
	{
		for (auto &b : privateBuffers)
			delete b.p;
		for (auto &i : privateImages)
			delete i.p;
		for (auto &p : privatePipelines)
			delete p.p;
	}

	ResourceBank globalResource(nullptr);

	struct ShaderModule
	{
		std::string filename;
		VkShaderModule v;
	};
	static std::vector<ShaderModule> shaderModules;

	Stage::Stage(Pipeline *_parent)
	{
		parent = _parent;
	}

	void Stage::create()
	{
		// format the shader path, so that they can reuse if them refer the same one
		auto path = std::experimental::filesystem::canonical(parent->filepath + "/" + filename + ".spv").string();
		for (auto &m : shaderModules)
		{
			if (m.filename == path)
			{
				module = m.v;
				return;
			}
		}

		ShaderModule shaderModule;
		shaderModule.filename = path;

		OnceFileBuffer file(path);
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = file.length;
		shaderModuleCreateInfo.pCode = (uint32_t*)file.data;

		device.cs.lock();
		auto res = vkCreateShaderModule(device.v, &shaderModuleCreateInfo, nullptr, &shaderModule.v);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		shaderModules.push_back(shaderModule);

		module = shaderModule.v;
	}

	Stage::~Stage()
	{
		for (auto it = shaderModules.begin(); it != shaderModules.end(); it++)
		{
			if (it->v == module)
			{
				shaderModules.erase(it);

				device.cs.lock();
				vkDestroyShaderModule(device.v, module, nullptr);
				device.cs.unlock();

				return;
			}
		}
	}

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;
	};
	static std::vector<DescriptorSetLayout> descriptorSetLayouts;

	struct PipelineLayout
	{
		VkDescriptorSetLayout descriptorLayout;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout v;
	};
	std::vector<PipelineLayout> pipelineLayouts;

	Pipeline::Pipeline()
	{
		pResource = &globalResource;
	}

	void Pipeline::create(const std::string &_filename, VkPipelineVertexInputStateCreateInfo *pVertexInputState, VkRenderPass renderPass, std::uint32_t subpassIndex)
	{
		pipelineLoadXML<Pipeline, Stage>(this, _filename);

		m_pVertexInputState = pVertexInputState;

		m_renderPass = renderPass;
		m_subpassIndex = subpassIndex;

		if (cx == -1)
			cx = tke::resCx;
		if (cy == -1)
			cy = tke::resCy;

		if (cx == 0 && cy == 0)
		{
			m_dynamics.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			m_dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
		}

		switch (primitive_topology)
		{
		case PrimitiveTopology::triangle_list:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case PrimitiveTopology::line_list:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case PrimitiveTopology::line_strip:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case PrimitiveTopology::patch_list:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			break;
		}

		switch (polygon_mode)
		{
		case PolygonMode::fill:
			vkPolygonMode = VK_POLYGON_MODE_FILL;
			break;
		case PolygonMode::line:
			vkPolygonMode = VK_POLYGON_MODE_LINE;
			break;
		}

		switch (cull_mode)
		{
		case CullMode::none:
			vkCullMode = VK_CULL_MODE_NONE;
			break;
		case CullMode::front:
			vkCullMode = VK_CULL_MODE_FRONT_BIT;
			break;
		case CullMode::back:
			vkCullMode = VK_CULL_MODE_BACK_BIT;
			break;
		case CullMode::front_and_back:
			vkCullMode = VK_CULL_MODE_FRONT_AND_BACK;
			break;
		}

		vkBlendAttachments.clear();
		vkDescriptors.clear();
		vkPushConstantRanges.clear();
		vkStages.clear();

		for (auto &b : blendAttachments)
		{
			VkPipelineColorBlendAttachmentState s = {};
			s.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			s.blendEnable = b.enable;
			s.srcColorBlendFactor = _vkBlendFactor(b.src_color);
			s.dstColorBlendFactor = _vkBlendFactor(b.dst_color);
			s.srcAlphaBlendFactor = _vkBlendFactor(b.src_alpha);
			s.dstAlphaBlendFactor = _vkBlendFactor(b.dst_alpha);
			vkBlendAttachments.push_back(s);
		}

		for (auto s : stages)
		{
			if (!s) continue;

			s->create();

			VkPipelineShaderStageCreateInfo i = {};
			i.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			i.pName = "main";
			i.stage = (VkShaderStageFlagBits)vkStage(s->type);
			i.module = s->module;
			vkStages.push_back(i);

			for (auto &d : s->descriptors)
			{
				auto found = false;
				for (auto &b : vkDescriptors)
				{
					if (b.binding == d.binding)
					{
						b.stageFlags |= vkStage(s->type);
						found = true;
						break;
					}
				}
				if (found) continue;

				VkDescriptorSetLayoutBinding b = {};
				b.descriptorType = _vkDescriptorType(d.type);
				b.binding = d.binding;
				b.descriptorCount = d.count;
				b.stageFlags = vkStage(s->type);
				vkDescriptors.push_back(b);
			}
			for (auto &p : s->pushConstantRanges)
			{
				auto found = false;
				for (auto &r : vkPushConstantRanges)
				{
					if (r.offset == p.offset & r.size == p.size)
					{
						r.stageFlags |= vkStage(s->type);
						found = true;
						break;
					}
				}
				if (found) continue;

				VkPushConstantRange r = {};
				r.offset = p.offset;
				r.size = p.size;
				r.stageFlags = vkStage(s->type);
				vkPushConstantRanges.push_back(r);
			}
		}

		{
			bool found = false;
			for (auto &d : descriptorSetLayouts)
			{
				if (d.bindings.size() == vkDescriptors.size())
				{
					bool same = true;
					for (auto i = 0; i < d.bindings.size(); i++)
					{
						auto &binding = d.bindings[i];
						if (binding.binding != vkDescriptors[i].binding || binding.descriptorCount != vkDescriptors[i].descriptorCount ||
							binding.descriptorType != vkDescriptors[i].descriptorType || binding.stageFlags != vkDescriptors[i].stageFlags)
						{
							same = false;
							break;
						}
					}
					if (same)
					{
						m_descriptorSetLayout = d.v;
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				DescriptorSetLayout descriptorSetLayout;
				descriptorSetLayout.bindings.insert(descriptorSetLayout.bindings.begin(), vkDescriptors.begin(), vkDescriptors.end());

				VkDescriptorSetLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.bindingCount = vkDescriptors.size();
				info.pBindings = vkDescriptors.data();

				device.cs.lock();
				auto res = vkCreateDescriptorSetLayout(device.v, &info, nullptr, &descriptorSetLayout.v);
				assert(res == VK_SUCCESS);
				device.cs.unlock();

				descriptorSetLayouts.push_back(descriptorSetLayout);

				m_descriptorSetLayout = descriptorSetLayout.v;
			}
		}

		m_descriptorSet = descriptorPool.allocate(&m_descriptorSetLayout);

		{
			bool found = false;
			for (auto &p : pipelineLayouts)
			{
				if (p.descriptorLayout == m_descriptorSetLayout && p.pushConstantRanges.size() == vkPushConstantRanges.size())
				{
					bool same = true;
					for (auto i = 0; i < p.pushConstantRanges.size(); i++)
					{
						auto &pc = p.pushConstantRanges[i];
						if (pc.offset != vkPushConstantRanges[i].offset || pc.size != vkPushConstantRanges[i].size ||
							pc.stageFlags != vkPushConstantRanges[i].stageFlags)
						{
							same = false;
							break;
						}
					}
					if (same)
					{
						m_pipelineLayout = p.v;
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				PipelineLayout pipelineLayout;
				pipelineLayout.descriptorLayout = m_descriptorSetLayout;
				pipelineLayout.pushConstantRanges.insert(pipelineLayout.pushConstantRanges.begin(), vkPushConstantRanges.begin(), vkPushConstantRanges.end());

				VkPipelineLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				info.setLayoutCount = 1;
				info.pSetLayouts = &pipelineLayout.descriptorLayout;
				info.pushConstantRangeCount = pipelineLayout.pushConstantRanges.size();
				info.pPushConstantRanges = pipelineLayout.pushConstantRanges.data();

				device.cs.lock();
				auto res = vkCreatePipelineLayout(device.v, &info, nullptr, &pipelineLayout.v);
				assert(res == VK_SUCCESS);
				device.cs.unlock();

				pipelineLayouts.push_back(pipelineLayout);

				m_pipelineLayout = pipelineLayout.v;
			}
		}

		VkPipelineInputAssemblyStateCreateInfo assemblyState = {};
		assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyState.topology = vkPrimitiveTopology;
		assemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo tessState = {};
		tessState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessState.patchControlPoints = patch_control_points;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = depth_test;
		depthStencilState.depthWriteEnable = depth_write;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

		VkViewport viewport;
		viewport.width = (float)cx;
		viewport.height = (float)cy;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		viewport.x = 0;
		viewport.y = 0;

		VkRect2D scissor;
		scissor.extent.width = cx;
		scissor.extent.height = cy;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		viewportState.pViewports = &viewport;

		VkPipelineRasterizationStateCreateInfo rasterState = {};
		rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterState.polygonMode = vkPolygonMode;
		rasterState.cullMode = vkCullMode;
		rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterState.depthClampEnable = depth_clamp;
		rasterState.rasterizerDiscardEnable = VK_FALSE;
		rasterState.lineWidth = 1.f;
		rasterState.depthBiasEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendState = {};
		if (vkBlendAttachments.size() == 0)
		{
			VkPipelineColorBlendAttachmentState attachment = {};
			attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			vkBlendAttachments.push_back(attachment);
		}

		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.logicOpEnable = VK_FALSE;
		blendState.logicOp = VK_LOGIC_OP_COPY;
		blendState.attachmentCount = vkBlendAttachments.size();
		blendState.pAttachments = vkBlendAttachments.data();

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = m_dynamics.size();
		dynamicState.pDynamicStates = m_dynamics.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.stageCount = vkStages.size();
		pipelineInfo.pStages = vkStages.data();
		pipelineInfo.pVertexInputState = m_pVertexInputState;
		pipelineInfo.pInputAssemblyState = &assemblyState;
		pipelineInfo.pTessellationState = patch_control_points ? &tessState : nullptr;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pColorBlendState = &blendState;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = subpassIndex;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDynamicState = m_dynamics.size() ? &dynamicState : nullptr;

		device.cs.lock();
		auto res = vkCreateGraphicsPipelines(device.v, 0, 1, &pipelineInfo, nullptr, &m_pipeline);
		assert(res == VK_SUCCESS);
		device.cs.unlock();

		for (auto &link : links)
		{
			DescriptorType type = DescriptorType::null;

			if (link.binding == -1)
			{
				bool found = false;
				for (auto s : stages)
				{
					if (!s) continue;

					if (found) break;
					for (auto &d : s->descriptors)
					{
						if (d.name == link.descriptor_name)
						{
							link.binding = d.binding;
							type = d.type;
							found = true;
							break;
						}
					}
				}
				//assert(found);
			}
			else
			{
				bool found = false;
				for (auto s : stages)
				{
					if (!s) continue;

					if (found) break;
					for (auto &d : s->descriptors)
					{
						if (d.binding == link.binding)
						{
							type = d.type;
							found = true;
							break;
						}
					}
				}
				if (!found) continue;
				//assert(found);
			}

			switch (type)
			{
			case DescriptorType::uniform_buffer:
			{
				auto pUniformBuffer = (UniformBuffer*)pResource->getBuffer(link.resource_name);
				if (pUniformBuffer)
					descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, &pUniformBuffer->m_info, link.array_element);
				else
					printf("%s: unable to link resource %s (binding:%d, type:uniform buffer)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
				break;
			case DescriptorType::storage_buffer:
			{
				auto pStorageBuffer = (UniformBuffer*)pResource->getBuffer(link.resource_name);
				if (pStorageBuffer)
					descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, &pStorageBuffer->m_info, link.array_element);
				else
					printf("%s: unable to link resource %s (binding:%d, type:storage buffer)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
				break;
			case DescriptorType::storage_image:
			{
				auto pStorageImage = pResource->getImage(link.resource_name);
				if (pStorageImage)
					descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, pStorageImage->getInfo(0), link.array_element);
				else
					printf("%s: unable to link resource %s (binding:%d, type:storage image)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
				break;
			case DescriptorType::image_n_sampler:
			{
				auto pTexture = pResource->getImage(link.resource_name);
				if (pTexture)
				{
					VkSampler sampler = 0;
					switch (link.sampler)
					{
					case SamplerType::none:
						break;
					case SamplerType::plain:
						sampler = plainSampler;
						break;
					case SamplerType::plain_unnormalized:
						sampler = plainUnnormalizedSampler;
						break;
					case SamplerType::color:
						sampler = colorSampler;
						break;
					case SamplerType::color_border:
						sampler = colorBorderSampler;
						break;
					}
					descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, pTexture->getInfo(sampler), link.array_element);
				}
				else
					printf("%s: unable to link resource %s (binding:%d, type:combined image sampler)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
				break;
			}
		}

		descriptorPool.update();
	}

	Pipeline::~Pipeline()
	{
		for (auto it = pipelineLayouts.begin(); it != pipelineLayouts.end(); it++)
		{
			if (it->v == m_pipelineLayout)
			{
				pipelineLayouts.erase(it);

				device.cs.lock();
				vkDestroyPipelineLayout(device.v, m_pipelineLayout, nullptr);
				device.cs.unlock();

				break;
			}
		}

		for (auto it = descriptorSetLayouts.begin(); it != descriptorSetLayouts.end(); it++)
		{
			if (it->v == m_descriptorSetLayout)
			{
				descriptorSetLayouts.erase(it);

				device.cs.lock();
				vkDestroyDescriptorSetLayout(device.v, m_descriptorSetLayout, nullptr);
				device.cs.unlock();

				break;
			}
		}

		device.cs.lock();
		vkDestroyPipeline(device.v, m_pipeline, nullptr);
		device.cs.unlock();

		for (int i = 0; i < 5; i++)
			delete stages[i];
	}

	int Pipeline::descriptorPosition(const std::string &name)
	{
		for (auto s : stages)
		{
			if (!s) continue;

			for (auto &d : s->descriptors)
			{
				if (d.name == name)
					return d.binding;
			}
		}
		assert(false);
		return -1;
	}

	Drawcall::Drawcall() {}

	Drawcall::Drawcall(int vertexCount, int firstVertex, int instanceCount, int firstInstance)
	{
		vertex_count = vertexCount;
		first_vertex = firstVertex;
		instance_count = instanceCount;
		first_instance = firstInstance;
		type = DrawcallType::vertex;
	}

	Drawcall::Drawcall(int indexCount, int firstIndex, int vertexOffset, int instanceCount, int firstInstance)
	{
		index_count = indexCount;
		first_index = firstIndex;
		vertex_offset = vertexOffset;
		instance_count = instanceCount;
		first_instance = firstInstance;
		type = DrawcallType::index;
	}

	Drawcall::Drawcall(Model *p, int instanceCount, int firstInstance)
	{
		index_count = p->indices.size();
		first_index = p->indiceBase;
		vertex_offset = p->vertexBase;
		instance_count = instanceCount;
		first_instance = firstInstance;
		type = DrawcallType::index;
	}

	Drawcall::Drawcall(IndirectVertexBuffer *indirectVertexBuffer, uint32_t firstIndirect, uint32_t indirectCount)
	{
		m_indirectVertexBuffer = indirectVertexBuffer;
		first_indirect = firstIndirect;
		indirect_count = indirectCount;
		type = DrawcallType::indirect_vertex;
	}

	Drawcall::Drawcall(IndirectIndexBuffer *indirectIndexBuffer, uint32_t firstIndirect, uint32_t indirectCount)
	{
		m_indirectIndexBuffer = indirectIndexBuffer;
		first_indirect = firstIndirect;
		indirect_count = indirectCount;
		type = DrawcallType::indirect_index;
	}

	Drawcall::Drawcall(StageType stage, void *data, size_t size, size_t offset)
	{
		push_constant_stage = stage;
		push_constant_offset = offset;
		push_constant_size = size;
		push_constant_value = malloc(size);
		memcpy(push_constant_value, data, size);
		type = DrawcallType::push_constant;
	}

	Drawcall::~Drawcall()
	{
		delete push_constant_value;
	}

	void Drawcall::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);
	}

	void Drawcall::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);
	}

	Drawcall *DrawAction::findDrawcall(const std::string &n)
	{
		for (auto &c : drawcalls)
			if (c.name == n)
				return &c;
		return nullptr;
	}

	DrawAction::DrawAction() {}

	DrawAction::DrawAction(Pipeline *pipeline)
	{
		m_pipeline = pipeline;
		type = DrawActionType::draw_action;
	}

	DrawAction::DrawAction(PF_RenderFunc pRenderFunc)
	{
		m_pRenderFunc = pRenderFunc;
		type = DrawActionType::call_fuction;
	}

	void DrawAction::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);

		auto drawcallsNode = n->firstNode("drawcalls");
		if (drawcallsNode)
		{
			for (auto nn : drawcallsNode->children)
			{
				if (nn->name == "drawcall")
				{
					auto d = addDrawcall();
					d->loadFromAt(nn);
				}
			}
		}
	}

	void DrawAction::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);

		auto drawcallsNode = new AttributeTreeNode("drawcalls");
		n->children.push_back(drawcallsNode);
		for (auto &d : drawcalls)
		{
			auto n = new AttributeTreeNode("drawcall");
			drawcallsNode->children.push_back(n);
			d.saveToAt(n);
		}
	}

	void DrawAction::maintain(int row)
	{
		maintainList(drawcalls);
	}

	void DrawAction::preprocess(Pipeline* &currentPipeline)
	{
		if (m_pipeline)
		{
			if (!m_descriptorSet)
			{
				if (m_pipeline->m_descriptorSet)
					m_descriptorSet = m_pipeline->m_descriptorSet;
			}
		}

		if (m_pipeline && m_pipeline != currentPipeline)
			currentPipeline = m_pipeline;
	}

	Attachment::Attachment() {}

	Attachment::Attachment(Image *_image, VkClearValue _clearValue)
	{
		image = _image;
		if (_clearValue.color.float32[0] == 9999.f) return;
		loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		clearValue = _clearValue;
	}

	void Attachment::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);
	}

	void Attachment::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);
	}

	Dependency::Dependency() {}

	Dependency::Dependency(void *_target)
	{
		target = _target;
	}

	Dependency::Dependency(const std::string &_pass_name)
	{
		pass_name = _pass_name;
	}

	void Dependency::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);
	}

	void Dependency::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);
	}

	Attachment *RenderPass::findAttachment(const std::string &n)
	{
		for (auto &a : attachments)
			if (a.name == n)
				return &a;
		return nullptr;
	}

	DrawAction *RenderPass::findAction(const std::string &n)
	{
		for (auto &a : actions)
			if (a.name == n)
				return &a;
		return nullptr;
	}

	RenderPass::RenderPass() {}

	RenderPass::RenderPass(VkCommandBuffer cmd)
	{
		type = RenderPassType::call_secondary_cmd;
		secondaryCmd = cmd;
	}

	void RenderPass::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);

		auto attachmentNode = n->firstNode("attachments");
		if (attachmentNode)
		{
			for (auto n : attachmentNode->children)
			{
				if (n->name == "attachment")
				{
					auto attachment = addAttachment();
					attachment->loadFromAt(n);
				}
			}
		}
		auto dependenciesNode = n->firstNode("dependencies");
		if (dependenciesNode)
		{
			for (auto n : dependenciesNode->children)
			{
				if (n->name == "dependency")
				{
					auto dependency = addDependency();
					dependency->loadFromAt(n);
				}
			}
		}
		auto actionsNode = n->firstNode("actions");
		if (actionsNode)
		{
			for (auto n : actionsNode->children)
			{
				if (n->name == "action")
				{
					auto action = addAction();
					action->loadFromAt(n);
				}
			}
		}
	}

	void RenderPass::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);

		auto attachmentNode = new AttributeTreeNode("attachments");
		n->children.push_back(attachmentNode);
		for (auto &a : attachments)
		{
			auto n = new AttributeTreeNode("attachment");
			attachmentNode->children.push_back(n);
			a.saveToAt(n);
		}
		auto dependenciesNode = new AttributeTreeNode("dependencies");
		n->children.push_back(dependenciesNode);
		for (auto &d : dependencies)
		{
			auto n = new AttributeTreeNode("dependency");
			dependenciesNode->children.push_back(n);
			d.saveToAt(n);
		}
		auto actionsNode = new AttributeTreeNode("actions");
		n->children.push_back(actionsNode);
		for (auto &a : actions)
		{
			auto n = new AttributeTreeNode("action");
			actionsNode->children.push_back(n);
			a.saveToAt(n);
		}
	}

	void RenderPass::maintain(int row)
	{
		switch (row)
		{
		case (int)RenderPassElement::eAction:
			maintainList(actions);
			break;
		case (int)RenderPassElement::eAttachment:
			maintainList(attachments);
			break;
		case (int)RenderPassElement::eDependency:
			maintainList(dependencies);
			break;
		}
	}

	RenderPass *Renderer::findRenderPass(const std::string &n)
	{
		for (auto &p : passes)
			if (p.name == n)
				return &p;
		return nullptr;
	}

	Renderer::Renderer(int _cx, int _cy)
		:resource(&globalResource)
	{
		cx = _cx;
		cy = _cy;
	}

	Renderer::~Renderer()
	{
		destroyFramebuffer(vkFramebuffer[0]);
		destroyFramebuffer(vkFramebuffer[1]);
		destroyRenderPass(vkRenderPass);
	}

	void Renderer::loadXML(const std::string &_filename)
	{
		filename = _filename;

		AttributeTree at("renderer");
		at.loadXML(filename);

		at.obtainFromAttributes(this, b);

		auto buffersNode = at.firstNode("buffers");
		if (buffersNode)
		{
			for (auto c : buffersNode->children)
			{
				if (c->name == "buffer")
				{
					UniformBufferInfo b;
					c->obtainFromAttributes(&b, b.b);
					resource.privateBuffers.push_back(b);
				}
			}
		}

		auto imagesNode = at.firstNode("images");
		if (imagesNode)
		{
			for (auto c : imagesNode->children)
			{
				if (c->name == "image")
				{
					ImageInfo i;
					c->obtainFromAttributes(&i, i.b);
					resource.privateImages.push_back(i);
				}
			}
		}

		auto pipelinesNode = at.firstNode("pipelines");
		if (pipelinesNode)
		{
			for (auto c : pipelinesNode->children)
			{
				if (c->name == "pipeline")
				{
					PipelineInfo p;
					c->obtainFromAttributes(&p, p.b);
					resource.privatePipelines.push_back(p);
				}
			}
		}

		auto passesNode = at.firstNode("passes");
		if (passesNode)
		{
			for (auto c : passesNode->children)
			{
				if (c->name == "pass")
				{
					auto pass = addPass();
					pass->loadFromAt(c);
				}
			}
		}

		for (auto &p : passes)
		{
			for (auto &d : p.dependencies)
			{
				for (auto &pp : passes)
				{
					if (d.pass_name == pp.name)
					{
						d.target = &pp;
						break;
					}
				}
			}
		}
	}

	void Renderer::saveXML()
	{
		for (auto &p : passes)
		{
			for (auto &d : p.dependencies)
			{
				if (d.target)
				{
					auto pp = (RenderPass*)d.target;
					d.pass_name = pp->name;
				}
			}
		}

		AttributeTree at("renderer");

		at.addAttributes(this, b);

		auto passesNode = new AttributeTreeNode("passes");
		at.children.push_back(passesNode);
		for (auto &p : passes)
		{
			auto n = new AttributeTreeNode("pass");
			passesNode->children.push_back(n);
			p.saveToAt(n);
		}

		at.saveXML(filename);
	}

	void Renderer::maintain(int row)
	{
		maintainList(passes);
	}

	void renderer_pushImage(Renderer *r, Attachment *ai)
	{
		auto view = ai->image->getView(VkImageAspectFlags(ai->aspect), ai->level, 1, ai->layer, 1);

		auto index = 0;
		for (; index < r->vkViews[0].size(); index++)
		{
			if (r->vkViews[0][index] == view)
			{
				ai->index = index;
				return;
			}
		}
		VkAttachmentDescription attachment;
		switch (ai->image->type)
		{
		case Image::eColor:
			attachment = colorAttachmentDesc(ai->image->m_format, ai->loadOp);
			break;
		case Image::eSwapchain:
			attachment = swapchainAttachmentDesc(ai->loadOp);
			break;
		case Image::eDepth:
			attachment = depthAttachmentDesc(ai->image->m_format, ai->loadOp);
			break;
		case Image::eDepthStencil:
			assert(0);
			//attachment = Vk::depthAttachmentDesc(ai->image->m_format, ai->loadOp);
			break;
		}
		r->vkAttachments.push_back(attachment);
		r->vkViews[0].push_back(view);
		if (r->containSwapchain)
		{
			if (ai->image->type == Image::eSwapchain)
				view = ai->image[1].getView(VkImageAspectFlags(ai->aspect), ai->layer, 1, ai->layer, 1);
			r->vkViews[1].push_back(view);
		}
		r->vkClearValues.push_back(ai->clearValue);
		ai->index = index;
	}

	void Renderer::getDescriptorSets()
	{
		currentPipeline = initPipeline;

		for (auto &pass : passes)
		{
			for (auto &action : pass.actions)
				action.preprocess(currentPipeline);
		}

		currentPipeline = nullptr;
	}

	void Renderer::execute(VkCommandBuffer cmd, int index)
	{
		currentVertexBuffer = initVertexBuffer;
		currentIndexBuffer = initIndexBuffer;
		currentPipeline = initPipeline;
		currentDescriptorSet = initDescriptorSet;

		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo(vkRenderPass, vkFramebuffer[index], cx, cy, vkClearValues.size(), vkClearValues.data()), VK_SUBPASS_CONTENTS_INLINE);

		if (currentVertexBuffer)
			currentVertexBuffer->bind(cmd);

		if (currentIndexBuffer)
			currentIndexBuffer->bind(cmd);

		bool firstPass = true;
		for (auto &pass : passes)
		{
			switch (pass.type)
			{
			case RenderPassType::draw_action:
				if (!firstPass) vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
				for (auto &action : pass.actions)
				{
					if (!action.show) continue;

					if (action.cx || action.cy)
					{
						VkViewport viewport;
						viewport.x = 0;
						viewport.y = 0;
						viewport.width = action.cx;
						viewport.height = action.cy;
						viewport.minDepth = 0.0f;
						viewport.maxDepth = 1.0f;
						vkCmdSetViewport(cmd, 0, 1, &viewport);

						VkRect2D scissor;
						scissor.offset.x = 0;
						scissor.offset.y = 0;
						scissor.extent.width = action.cx;
						scissor.extent.height = action.cy;
						vkCmdSetScissor(cmd, 0, 1, &scissor);
					}
					switch (action.type)
					{
					case DrawActionType::draw_action:
						if (action.m_pipeline && action.m_pipeline != currentPipeline)
						{
							vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, action.m_pipeline->m_pipeline);
							currentPipeline = action.m_pipeline;
						}
						if (action.m_vertexBuffer && action.m_vertexBuffer != currentVertexBuffer)
						{
							action.m_vertexBuffer->bind(cmd);
							currentVertexBuffer = action.m_vertexBuffer;
						}
						if (action.m_indexBuffer && action.m_indexBuffer != currentIndexBuffer)
						{
							action.m_indexBuffer->bind(cmd);
							currentIndexBuffer = action.m_indexBuffer;
						}
						if (action.m_descriptorSet && action.m_descriptorSet != currentDescriptorSet)
						{
							vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, action.m_pipeline->m_pipelineLayout, 0, 1, &action.m_descriptorSet, 0, nullptr);
							currentDescriptorSet = action.m_descriptorSet;
						}

						for (auto &drawcall : action.drawcalls)
						{
							switch (drawcall.type)
							{
							case DrawcallType::vertex:
								vkCmdDraw(cmd, drawcall.vertex_count, drawcall.instance_count, drawcall.first_vertex, drawcall.first_instance);
								break;
							case DrawcallType::index:
								vkCmdDrawIndexed(cmd, drawcall.index_count, drawcall.instance_count, drawcall.first_index, drawcall.vertex_offset, drawcall.first_instance);
								break;
							case DrawcallType::indirect_vertex:
								vkCmdDrawIndirect(cmd, drawcall.m_indirectVertexBuffer->m_buffer, drawcall.first_indirect * sizeof VkDrawIndirectCommand, drawcall.indirect_count, sizeof VkDrawIndirectCommand);
								break;
							case DrawcallType::indirect_index:
								vkCmdDrawIndexedIndirect(cmd, drawcall.m_indirectIndexBuffer->m_buffer, drawcall.first_indirect * sizeof VkDrawIndexedIndirectCommand, drawcall.indirect_count, sizeof VkDrawIndexedIndirectCommand);
								break;
							case DrawcallType::push_constant:
								vkCmdPushConstants(cmd, currentPipeline->m_pipelineLayout, vkStage(drawcall.push_constant_stage), drawcall.push_constant_offset, drawcall.push_constant_size, drawcall.push_constant_value);
								break;
							}
						}
						break;
					case DrawActionType::call_fuction:
						if (action.m_pRenderFunc)
							action.m_pRenderFunc(cmd);
						break;
					}
				}
				break;
			case RenderPassType::call_secondary_cmd:
				if (!firstPass) vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
				if (pass.secondaryCmd) vkCmdExecuteCommands(cmd, 1, &pass.secondaryCmd);
				break;
			}
			firstPass = false;
		}

		vkCmdEndRenderPass(cmd);

		currentVertexBuffer = nullptr;
		currentIndexBuffer = nullptr;
		currentPipeline = nullptr;
		currentDescriptorSet = 0;
	}

	void Renderer::setup()
	{
		if (cx == -1)
			cx = tke::resCx;
		if (cy == -1)
			cy = tke::resCy;

		for (auto &b : resource.privateBuffers)
		{
			b.p = new UniformBuffer(b.size);
			resource.setBuffer(b.p, b.name);
		}

		for (auto &i : resource.privateImages)
		{
			if (i.cx == -1)
				i.cx = tke::resCx;
			if (i.cy == -1)
				i.cy = tke::resCy;

			if (i.file_name != "")
				i.p = createImage(enginePath + i.file_name, i.sRGB);
			else
				i.p = new Image(i.cx, i.cy, vkFormat(i.format), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			resource.setImage(i.p, i.name);
		}

		for (auto &p : resource.privatePipelines)
		{
			p.p = new Pipeline;
			p.p->pResource = &resource;
			resource.setPipeline(p.p, p.name);
		}

		if (vertex_buffer_name!= "")
			initVertexBuffer = (VertexBuffer*)resource.getBuffer(vertex_buffer_name);
		if (index_buffer_name != "")
			initIndexBuffer = (IndexBuffer*)resource.getBuffer(index_buffer_name);

		for (auto &p : passes)
		{
			for (auto &a : p.attachments)
			{
				if (a.image_name != "")
					a.image = resource.getImage(a.image_name);
				if (a.image->type == Image::eSwapchain)
					containSwapchain = true;
				if (a.clear)
				{
					a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					if (a.image->isColorType())
					{
						a.clearValue.color.float32[0] = a.clear_r;
						a.clearValue.color.float32[1] = a.clear_g;
						a.clearValue.color.float32[2] = a.clear_b;
						a.clearValue.color.float32[3] = a.clear_a;
					}
					else
					{
						a.clearValue.depthStencil.depth = a.clear_depth;
						a.clearValue.depthStencil.stencil = a.clear_stencil;
					}
				}
			}
		}

		std::vector<VkSubpassDescription> vkSubpasses;
		std::vector<VkSubpassDependency> vkDependencies;

		std::vector<std::vector<VkAttachmentReference>> vkRefLists(passes.size());

		int subpassIndex = 0;
		for (auto &pass : passes)
		{
			pass.index = subpassIndex;

			for (auto &a : pass.attachments)
				renderer_pushImage(this, &a);

			VkSubpassDescription desc = {};
			desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			bool hasDepthStencil = false;
			for (auto &a : pass.attachments)
			{
				if (a.image->isColorType())
				{
					vkRefLists[subpassIndex].push_back({ (unsigned int)a.index, VK_IMAGE_LAYOUT_GENERAL });
					desc.colorAttachmentCount++;
				}
				else
				{
					vkRefLists[subpassIndex].insert(vkRefLists[subpassIndex].begin(), { (unsigned int)a.index, VK_IMAGE_LAYOUT_GENERAL });
					hasDepthStencil = true;
				}
			}

			if (desc.colorAttachmentCount > 0)
				desc.pColorAttachments = hasDepthStencil ? &vkRefLists[subpassIndex][1] : vkRefLists[subpassIndex].data();
			if (hasDepthStencil)
				desc.pDepthStencilAttachment = vkRefLists[subpassIndex].data();

			vkSubpasses.push_back(desc);

			for (auto &dependency : pass.dependencies)
			{
				auto pass = (RenderPass*)dependency.target;
				vkDependencies.push_back(subpassDependency(pass->index, subpassIndex));
			}

			switch (pass.type)
			{
			case RenderPassType::draw_action:
				for (auto &action : pass.actions)
				{
					switch (action.type)
					{
					case DrawActionType::draw_action:
						if (action.pipeline_name != "")
						{
							action.m_pipeline = resource.getPipeline(action.pipeline_name.c_str());
							if (action.m_pipeline)
							{
								for (auto &p : resource.privatePipelines)
								{
									if (p.name == action.pipeline_name)
										p.subpassIndex = subpassIndex;
								}
							}
						}
						for (auto &drawcall : action.drawcalls)
						{
							if (drawcall.indirect_vertex_buffer_name != "")
								drawcall.m_indirectVertexBuffer = (IndirectVertexBuffer*)resource.getBuffer(drawcall.indirect_vertex_buffer_name);
							if (drawcall.indirect_index_buffer_name != "")
								drawcall.m_indirectIndexBuffer = (IndirectIndexBuffer*)resource.getBuffer(drawcall.indirect_index_buffer_name);
							if (drawcall.model_name != "")
							{
								auto p = resource.getModel(drawcall.model_name);
								if (p)
								{
									drawcall.index_count = p->indices.size();
									drawcall.first_index = p->indiceBase;
									drawcall.vertex_offset = p->vertexBase;
								}
							}
						}
						break;
					case DrawActionType::call_fuction:
						break;
					}
				}
				break;
			case RenderPassType::call_secondary_cmd:
				if (pass.secondary_cmd_name != "")
					pass.secondaryCmd = resource.getCmd(pass.secondary_cmd_name.c_str());
				break;
			}

			subpassIndex++;
		}

		vkRenderPass = createRenderPass(vkAttachments.size(), vkAttachments.data(),
			vkSubpasses.size(), vkSubpasses.data(), vkDependencies.size(), vkDependencies.data());

		vkFramebuffer[0] = createFramebuffer(cx, cy, vkRenderPass, vkViews[0]);
		if (containSwapchain)
			vkFramebuffer[1] = createFramebuffer(cx, cy, vkRenderPass, vkViews[1]);

		for (auto &p : resource.privatePipelines)
			p.p->create(enginePath + p.file_name, p.vertex_input_type == VertexInputType::zero ? &zeroVertexInputState : &vertexInputState, vkRenderPass, p.subpassIndex);

		getDescriptorSets();
	}
}
