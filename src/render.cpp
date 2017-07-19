#include <filesystem>
#include <regex>
#include <assert.h>
#include <sstream>
#include <stack>

#include "render.h"
#include "core.h"

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
		info.pWaitSemaphores = &waitSemaphore;
		info.commandBufferCount = count;
		info.pCommandBuffers = cmds;
		info.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		info.pSignalSemaphores = &signalSemaphore;

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

	CommandBuffer::CommandBuffer(CommandPool *_pool, VkCommandBufferLevel level)
		:pool(_pool)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = level;
		info.commandPool = pool->v;
		info.commandBufferCount = 1;

		device.mtx.lock();
		auto res = vkAllocateCommandBuffers(device.v, &info, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	CommandBuffer::~CommandBuffer()
	{
		device.mtx.lock();
		vkFreeCommandBuffers(device.v, pool->v, 1, &v);
		device.mtx.unlock();
	}

	void CommandBuffer::reset()
	{
		vkResetCommandBuffer(v, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void CommandBuffer::begin(VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo *pInheritance)
	{
		currentPipeline = nullptr;

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = flags;
		info.pInheritanceInfo = pInheritance;
		auto res = vkBeginCommandBuffer(v, &info);
		assert(res == VK_SUCCESS);
	}

	void CommandBuffer::end()
	{
		auto res = vkEndCommandBuffer(v);
		assert(res == VK_SUCCESS);
	}

	void CommandBuffer::beginRenderPass(RenderPass *renderPass, Framebuffer *fb, VkClearValue *pClearValue)
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = renderPass->v;
		info.framebuffer = fb->v;
		info.renderArea.extent.width = fb->cx;
		info.renderArea.extent.height = fb->cy;
		info.clearValueCount = renderPass->clearValues.size();
		info.pClearValues = pClearValue ? pClearValue : renderPass->clearValues.data();

		vkCmdBeginRenderPass(v, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::nextSubpass(VkSubpassContents contents)
	{
		vkCmdNextSubpass(v, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::endRenderPass()
	{
		vkCmdEndRenderPass(v);
	}

	void CommandBuffer::setViewportAndScissor(int cx, int cy)
	{
		VkViewport viewport;
		viewport.width = (float)cx;
		viewport.height = (float)cy;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		viewport.x = 0;
		viewport.y = 0;
		vkCmdSetViewport(v, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.extent.width = cx;
		scissor.extent.height = cy;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(v, 0, 1, &scissor);
	}

	void CommandBuffer::setScissor(int x, int y, int cx, int cy)
	{
		VkRect2D scissor;
		scissor.extent.width = cx;
		scissor.extent.height = cy;
		scissor.offset.x = x;
		scissor.offset.y = y;
		vkCmdSetScissor(v, 0, 1, &scissor);
	}

	void CommandBuffer::bindVertexBuffer(VertexBuffer *b)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(v, 0, 1, &b->v, offsets);
	}

	void CommandBuffer::bindVertexBuffer(OnceVertexBuffer *b)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(v, 0, 1, &b->v, offsets);
	}

	void CommandBuffer::bindIndexBuffer(IndexBuffer *b)
	{
		vkCmdBindIndexBuffer(v, b->v, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::bindIndexBuffer(OnceIndexBuffer *b)
	{
		vkCmdBindIndexBuffer(v, b->v, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::bindPipeline(Pipeline *p)
	{
		currentPipeline = p;
		vkCmdBindPipeline(v, VK_PIPELINE_BIND_POINT_GRAPHICS, p->pipeline);
	}

	void CommandBuffer::bindDescriptorSet()
	{
		vkCmdBindDescriptorSets(v, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipelineLayout->v, 0, 1, &currentPipeline->descriptorSet->v, 0, nullptr);
	}

	void CommandBuffer::bindDescriptorSet(VkDescriptorSet set)
	{
		vkCmdBindDescriptorSets(v, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipelineLayout->v, 0, 1, &set, 0, nullptr);
	}

	void CommandBuffer::execSecondaryCmd(VkCommandBuffer cmd)
	{
		vkCmdExecuteCommands(v, 1, &cmd);
	}

	void CommandBuffer::pushConstant(StageType stage, int offset, int size, void *src)
	{
		vkCmdPushConstants(v, currentPipeline->pipelineLayout->v, vkStage(stage), offset, size, src);
	}

	void CommandBuffer::draw(int vertexCount, int firstVertex, int instanceCount, int firstInstance)
	{
		vkCmdDraw(v, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::drawIndex(int indexCount, int firstIndex, int vertexOffset, int instanceCount, int firstInstance)
	{
		vkCmdDrawIndexed(v, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::drawIndirect(IndirectVertexBuffer *b, int count, int offset)
	{
		vkCmdDrawIndirect(v, b->v, offset * sizeof VkDrawIndirectCommand, count, sizeof VkDrawIndirectCommand);
	}

	void CommandBuffer::drawIndirectIndex(IndirectIndexBuffer *b, int count, int offset)
	{
		vkCmdDrawIndexedIndirect(v, b->v, offset * sizeof VkDrawIndexedIndirectCommand, count, sizeof VkDrawIndexedIndirectCommand);
	}

	void CommandBuffer::waitEvents(size_t count, VkEvent *e)
	{
		vkCmdWaitEvents(v, count, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
	}

	void CommandBuffer::setEvent(VkEvent e)
	{
		vkCmdSetEvent(v, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	void CommandBuffer::resetEvent(VkEvent e)
	{
		vkCmdResetEvent(v, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	CommandPool::CommandPool()
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = 0;
		device.mtx.lock();
		auto res = vkCreateCommandPool(device.v, &info, nullptr, &v);
		device.mtx.unlock();
		assert(res == VK_SUCCESS);
	}

	CommandPool::~CommandPool()
	{
		device.mtx.lock();
		vkDestroyCommandPool(device.v, v, nullptr);
		device.mtx.unlock();
	}

	CommandBuffer *CommandPool::begineOnce()
	{
		auto cb = new CommandBuffer(this);
		cb->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return cb;
	}

	void CommandPool::endOnce(CommandBuffer *cb)
	{
		cb->end();

		graphicsQueue.submit(1, &cb->v);
		graphicsQueue.waitIdle();

		delete cb;
	}

	void CommandPool::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset, size_t dstOffset)
	{
		auto cb = begineOnce();
		VkBufferCopy region = { srcOffset, dstOffset, size};
		vkCmdCopyBuffer(cb->v, srcBuffer, dstBuffer, 1, &region);
		endOnce(cb);
	}

	void CommandPool::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges)
	{
		auto cb = begineOnce();
		vkCmdCopyBuffer(cb->v, srcBuffer, dstBuffer, count, ranges);
		endOnce(cb);
	}

	void CommandPool::updateBuffer(void *data, size_t size, Buffer *stagingBuffer, VkBuffer &Buffer)
	{
		void* map = stagingBuffer->map(0, size);
		memcpy(map, data, size);
		stagingBuffer->unmap();

		copyBuffer(stagingBuffer->v, Buffer, size);
	}

	void CommandPool::copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
	{
		auto cb = begineOnce();
		VkImageCopy region = {};
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.layerCount = 1;
		region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.layerCount = 1;
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;
		vkCmdCopyImage(cb->v, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		endOnce(cb);
	}

	CommandPool *commandPool = nullptr;

	DescriptorSet::DescriptorSet(DescriptorPool *_pool, DescriptorSetLayout *_layout)
		:pool(_pool), layout(_layout)
	{
		VkDescriptorSetAllocateInfo descriptorSetInfo = {};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = pool->v;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = &layout->v;

		device.mtx.lock();
		auto res = vkAllocateDescriptorSets(device.v, &descriptorSetInfo, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	DescriptorSet::~DescriptorSet()
	{
		device.mtx.lock();
		auto res = vkFreeDescriptorSets(device.v, pool->v, 1, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	void DescriptorSet::setBuffer(int binding, int index, Buffer *buffer)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		VkDescriptorBufferInfo info;
		info.offset = 0;
		info.buffer = buffer->v;
		info.range = buffer->size;
		write.pBufferInfo = &info;

		device.mtx.lock();
		vkUpdateDescriptorSets(device.v, 1, &write, 0, nullptr);
		device.mtx.unlock();
	}

	void DescriptorSet::setImage(int binding, int index, Image *image, VkSampler sampler, VkImageAspectFlags aspect, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		VkDescriptorImageInfo info;
		info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		info.imageView = image->getView(aspect, baseLevel, levelCount, baseLayer, layerCount);
		info.sampler = sampler;
		write.pImageInfo = &info;

		device.mtx.lock();
		vkUpdateDescriptorSets(device.v, 1, &write, 0, nullptr);
		device.mtx.unlock();
	}

	DescriptorPool::DescriptorPool()
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
		device.mtx.lock();
		auto res = vkCreateDescriptorPool(device.v, &descriptorPoolInfo, nullptr, &v);
		device.mtx.unlock();
		assert(res == VK_SUCCESS);
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(device.v, v, nullptr);
	}

	DescriptorPool *descriptorPool = nullptr;

	Framebuffer::~Framebuffer()
	{
		device.mtx.lock();
		vkDestroyFramebuffer(device.v, v, nullptr);
		device.mtx.unlock();
	}

	std::vector<Framebuffer*> framebuffers;

	Framebuffer *getFramebuffer(Image *i, RenderPass *renderPass, int level)
	{
		auto view = i->getView(0, level);
		return getFramebuffer(i->getWidth(level), i->getHeight(level), renderPass, 1, &view);
	}

	Framebuffer *getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views)
	{
		for (auto f : framebuffers)
		{
			if (f->views.size() == viewCount)
			{
				bool same = true;
				for (auto i = 0; i < f->views.size(); i++)
				{
					if (f->views[i] != views[i])
					{
						same = false;
						break;
					}
				}
				if (same)
				{
					f->refCount++;
					return f;
				}
			}
		}

		auto f = new Framebuffer;
		f->cx = cx;
		f->cy = cy;
		for (int i = 0; i < viewCount; i++)
			f->views.push_back(views[i]);

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = cx;
		info.height = cy;
		info.layers = 1;
		info.renderPass = renderPass->v;
		info.attachmentCount = viewCount;
		info.pAttachments = views;

		device.mtx.lock();
		auto res = vkCreateFramebuffer(device.v, &info, nullptr, &f->v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		framebuffers.push_back(f);
		return f;
	}

	void releaseFramebuffer(Framebuffer *f)
	{
		f->refCount--;
		if (f->refCount == 0)
		{
			for (auto it = framebuffers.begin(); it != framebuffers.end(); it++)
			{
				if (*it == f)
				{
					framebuffers.erase(it);
					delete f;
					return;
				}
			}
		}
	}

	VkFormat swapchainFormat;
	VkSampler plainSampler;
	VkSampler plainUnnormalizedSampler;
	VkSampler colorSampler;
	VkSampler colorBorderSampler;

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

	RenderPass::RenderPass(int attachmentCount, VkAttachmentDescription *pAttachments, int subpassCount, VkSubpassDescription *pSubpasses, int dependencyCount, VkSubpassDependency *pDependencies)
	{
		for (int i = 0; i < attachmentCount; i++)
		{
			if (pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
				pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				clearValues.push_back({});
			else
				clearValues.push_back({ 1, 0.f });
		}

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = attachmentCount;
		info.pAttachments = pAttachments;
		info.subpassCount = subpassCount;
		info.pSubpasses = pSubpasses;
		info.dependencyCount = dependencyCount;
		info.pDependencies = pDependencies;

		device.mtx.lock();
		auto res = vkCreateRenderPass(device.v, &info, nullptr, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	RenderPass::~RenderPass()
	{
		device.mtx.lock();
		vkDestroyRenderPass(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
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
		bufferInfo.size = p->size;
		bufferInfo.usage = p->usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		device.mtx.lock();

		res = vkCreateBuffer(device.v, &bufferInfo, nullptr, &p->v);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.v, p->v, &memRequirements);

		assert(p->size <= memRequirements.size);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, p->memoryProperty);

		res = vkAllocateMemory(device.v, &allocInfo, nullptr, &p->memory);
		assert(res == VK_SUCCESS);

		res = vkBindBufferMemory(device.v, p->v, p->memory, 0);
		assert(res == VK_SUCCESS);

		device.mtx.unlock();
	}

	static void buffer_copy(Buffer *p, void *data)
	{
		StagingBuffer stagingBuffer(p->size);

		void* map = stagingBuffer.map(0, p->size);
		memcpy(map, data, p->size);
		stagingBuffer.unmap();
		commandPool->copyBuffer(stagingBuffer.v, p->v, p->size);
	}

	static void buffer_destroy(Buffer *p)
	{
		device.mtx.lock();
		vkFreeMemory(device.v, p->memory, nullptr);
		vkDestroyBuffer(device.v, p->v, nullptr);
		device.mtx.unlock();
	}

	Buffer::Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty)
	{
		size = _size;
		usage = _usage;
		memoryProperty = _memoryProperty;

		buffer_create(this);
	}

	Buffer::~Buffer()
	{
		buffer_destroy(this);
	}

	void Buffer::recreate(size_t _size, void *data)
	{
		buffer_destroy(this);
		size = _size;
		buffer_create(this);
		if (data)
			buffer_copy(this, data);
	}

	void Buffer::update(void *data, StagingBuffer *stagingBuffer, size_t _size)
	{
		if (_size == 0) _size = size;
		commandPool->updateBuffer(data, _size, stagingBuffer, v);
	}

	void *Buffer::map(size_t offset, size_t _size)
	{
		void *map;
		device.mtx.lock();
		auto res = vkMapMemory(device.v, memory, offset, _size, 0, &map);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
		return map;
	}

	void Buffer::unmap()
	{
		device.mtx.lock();
		vkUnmapMemory(device.v, memory);
		device.mtx.unlock();
	}

	StagingBuffer::StagingBuffer(size_t _size)
		:Buffer(_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT/*dst usage for pick up*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
	}

	UniformBuffer::UniformBuffer(size_t _size)
		:Buffer(_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
	}

	VertexBuffer::VertexBuffer(size_t _size, void *data)
		: Buffer(_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		if (data)
			buffer_copy(this, data);
	}

	IndexBuffer::IndexBuffer(size_t _size, void *data)
		: Buffer(_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		if (data)
			buffer_copy(this, data);
	}

	OnceVertexBuffer::OnceVertexBuffer(size_t _size, void *data)
		: Buffer(_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
		if (data)
			buffer_copy(this, data);
	}

	OnceIndexBuffer::OnceIndexBuffer(size_t _size, void *data)
		: Buffer(_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
		if (data)
			buffer_copy(this, data);
	}

	IndirectVertexBuffer::IndirectVertexBuffer(size_t _size)
		: Buffer(_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
	}

	IndirectIndexBuffer::IndirectIndexBuffer(size_t _size)
		: Buffer(_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
	}

	ImageView::ImageView(Image *_image)
		:image(_image)
	{
	}

	Image::Image(int w, int h, VkFormat _format, VkImageUsageFlags usage, int _level , void *_data, size_t _size, VkImageAspectFlags aspect)
	{
		cx = w;
		cy = h;
		level = _level;
		format = _format;
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
		imageInfo.mipLevels = level;
		imageInfo.arrayLayers = layer;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		device.mtx.lock();

		res = vkCreateImage(device.v, &imageInfo, nullptr, &v);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.v, v, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(device.v, &allocInfo, nullptr, &memory);
		assert(res == VK_SUCCESS);

		res = vkBindImageMemory(device.v, v, memory, 0);
		assert(res == VK_SUCCESS);

		device.mtx.unlock();

		size = memRequirements.size;

		if (_data && _size)
		{
			assert(_size <= size);
			fillData(0, _data, _size, aspect);
		}
		transitionLayout(0, aspect, VK_IMAGE_LAYOUT_GENERAL);
	}

	Image::Image(Type _type, VkImage _image, int w, int h, VkFormat _format)
	{
		type = _type;
		v = _image;
		cx = w;
		cy = h;
		format = _format;
	}

	Image::~Image()
	{
		delete data;
		device.mtx.lock();
		for (auto v : views)
			vkDestroyImageView(device.v, v->v, nullptr);
		if (type != Type::eSwapchain)
		{
			vkFreeMemory(device.v, memory, nullptr);
			vkDestroyImage(device.v, v, nullptr);
		}
		device.mtx.unlock();
	}

	void Image::transitionLayout(int _level, VkImageAspectFlags aspect, VkImageLayout _layout)
	{
		auto cb = commandPool->begineOnce();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = _layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = v;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = _level;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (layout == VK_IMAGE_LAYOUT_PREINITIALIZED) barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		else if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		if (_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		else if (_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		else if (_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		else if (_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(cb->v, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		commandPool->endOnce(cb);

		layout = _layout;
	}

	void Image::fillData(int _level, void *data, size_t _size, VkImageAspectFlags aspect)
	{
		transitionLayout(_level, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		StagingBuffer stagingBuffer(_size);

		void* map = stagingBuffer.map(0, _size);
		memcpy(map, data, _size);
		stagingBuffer.unmap();

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = getWidth(_level);
		region.imageExtent.height = getHeight(_level);
		region.imageExtent.depth = 1;
		region.bufferOffset = 0;

		auto cb = commandPool->begineOnce();
		vkCmdCopyBufferToImage(cb->v, stagingBuffer.v, v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		commandPool->endOnce(cb);
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

		for (auto view : views)
		{
			if (view->aspect == aspect && view->baseLevel == baseLevel && view->levelCount == levelCount &&
				view->baseLayer == baseLayer && view->layerCount == layerCount)
				return view->v;
		}

		auto view = new ImageView(this);
		view->aspect = aspect;
		view->baseLevel = baseLevel;
		view->levelCount = levelCount;
		view->baseLayer = baseLayer;
		view->layerCount = layerCount;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = v;
		info.viewType = viewType;
		info.format = format;
		info.subresourceRange.aspectMask = aspect;
		info.subresourceRange.baseMipLevel = baseLevel;
		info.subresourceRange.levelCount = levelCount;
		info.subresourceRange.baseArrayLayer = baseLayer;
		info.subresourceRange.layerCount = layerCount;

		device.mtx.lock();
		auto res = vkCreateImageView(device.v, &info, nullptr, &view->v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		views.push_back(view);
		return view->v;
	}

	int Image::getWidth(int _level) const
	{
		int w = cx;
		for (;;)
		{
			if (_level <= 0 || w < 2)
				break;
			_level--;
			w >>= 1;
		}
		return w;
	}

	int Image::getHeight(int _level) const
	{
		int h = cy;
		for (;;)
		{
			if (_level <= 0 || h < 2)
				break;
			_level--;
			h >>= 1;
		}
		return h;
	}

	unsigned char Image::getR(float _x, float _y)
	{
		if (!data || _x < 0.f || _y < 0.f || _x >= cx || _y >= cy)
			return 0;

		auto x = glm::fract(_x);
		int X = glm::floor(_x);
		auto y = glm::fract(_y);
		int Y = glm::floor(_y);

#define gd(a, b) (float)data[(a) * bytePerPixel + (b) * pitch]
		return glm::mix(glm::mix(gd(X, Y), gd(X + 1, Y), x), glm::mix(gd(X, Y + 1), gd(X + 1, Y + 1), x), y);
#undef gd
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

	ResourceBank::ResourceBank(ResourceBank *_parent)
	{
		parent = _parent;
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
		parent = _parent;
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
	static int _currentUboBinding = 0;
	void Stage::create()
	{
		std::vector<std::string> defines;
		for (auto &m : parent->shaderMacros)
		{
			if (((int)m.stage & (int)type))
				defines.push_back(m.value);
		}

		// format the shader path, so that they can reuse if them refer the same one
		auto stageFilename = std::experimental::filesystem::canonical(parent->filepath + "/" + filename).string();
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
				else if (states.top().first && std::regex_search(line, match, pattern = R"(layout\s*\(\s*binding\s*=\s*((TKE_UBO_BINDING)|([0-9]+))\s*\)\s*uniform\s+((sampler2D)\s+)?([\w_]+)\s*(\[\s*([0-9]+)\s*\])?)"))
				{
					auto name = match[6].str();
					Descriptor d;
					d.binding = -1;
					for (int i = 0; i < 5; i++)
					{
						if (parent->stages[i])
						{
							auto s = parent->stages[i];
							if (s->type != type && s->module)
							{
								for (auto &_d : s->module->descriptors)
								{
									if (_d.name == name)
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

					d.name = name;
					if (d.binding == -1)
					{
						d.binding = _currentUboBinding;
						d.type = match[5].matched ? DescriptorType::image_n_sampler : DescriptorType::uniform_buffer;
						d.count = match[8].matched ? std::stoi(match[8].str()) : 1;
						module->descriptors.push_back(d);
						_currentUboBinding++;
					}
					else
					{
						module->descriptors.push_back(d);
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

	static std::vector<DescriptorSetLayout*> descriptorSetLayouts;

	PipelineLayout::~PipelineLayout()
	{
		device.mtx.lock();
		vkDestroyPipelineLayout(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<PipelineLayout*> pipelineLayouts;

	Pipeline::Pipeline()
	{
		pResource = &globalResource;
	}

	void Pipeline::loadXML(const std::string &_filename)
	{
		filename = _filename;
		std::experimental::filesystem::path path(filename);
		filepath = path.parent_path().string();
		if (filepath == "")
			filepath = ".";

		AttributeTree at("pipeline");
		at.loadXML(filename);
		at.obtainFromAttributes(this, b);

		for (auto c : at.children)
		{
			if (c->name == "blend_attachment")
			{
				BlendAttachment ba;
				c->obtainFromAttributes(&ba, ba.b);
				blendAttachments.push_back(ba);
			}
			else if (c->name == "dynamic")
			{
				DynamicState s;
				c->obtainFromAttributes(&s, s.b);
				dynamicStates.push_back(s);
			}
			else if (c->name == "link")
			{
				LinkResource l;
				c->obtainFromAttributes(&l, l.b);
				links.push_back(l);
			}
			else if (c->name == "stage")
			{
				auto s = new Stage(this);
				c->obtainFromAttributes(s, s->b);
				std::experimental::filesystem::path path(s->filename);
				s->filepath = path.parent_path().string();
				if (s->filepath == "")
					s->filepath = ".";
				auto ext = path.extension().string();
				s->type = StageFlagByExt(ext);

				stages[StageIndexByType(s->type)] = s;
			}
			else if (c->name == "macro")
			{
				ShaderMacro m;
				c->obtainFromAttributes(&m, m.b);
				shaderMacros.push_back(m);
			}
		}
	}

	void Pipeline::saveXML(const std::string &filename)
	{
		AttributeTree at("pipeline");
		at.addAttributes(this, b);
		for (auto &b : blendAttachments)
		{
			auto n = new AttributeTreeNode("blend_attachment");
			n->addAttributes(&b, b.b);
			at.children.push_back(n);
		}
		for (auto &s : dynamicStates)
		{
			auto n = new AttributeTreeNode("dynamic");
			n->addAttributes(&s, s.b);
			at.children.push_back(n);
		}
		for (auto &l : links)
		{
			auto n = new AttributeTreeNode("link");
			n->addAttributes(&l, l.b);
			at.children.push_back(n);
		}
		for (int i = 0; i < 5; i++)
		{
			auto s = stages[i];
			if (!s) continue;

			auto n = new AttributeTreeNode("stage");
			n->addAttributes(s, s->b);
			at.children.push_back(n);
		}
		for (auto &m : shaderMacros)
		{
			auto n = new AttributeTreeNode("macro");
			n->addAttributes(&m, m.b);
			at.children.push_back(n);
		}

		at.saveXML(filename);
	}

	void Pipeline::setup(RenderPass *_renderPass, std::uint32_t _subpassIndex)
	{
		if (!pVertexInputState)
		{
			switch (vertex_input_type)
			{
			case VertexInputType::zero:
				pVertexInputState = &zeroVertexInputState;
				break;
			case VertexInputType::plain2d:
				pVertexInputState = &plain2dVertexInputState;
				break;
			case VertexInputType::normal:
				pVertexInputState = &vertexInputState;
				break;
			case VertexInputType::line:
				pVertexInputState = &lineVertexInputState;
				break;
			case VertexInputType::animated:
				pVertexInputState = &animatedVertexInputState;
				break;
			}
		}

		renderPass = _renderPass;
		subpassIndex = _subpassIndex;

		if (cx == -1)
			cx = tke::resCx;
		if (cy == -1)
			cy = tke::resCy;

		if (cx == 0 && cy == 0)
		{
			vkDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			vkDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		}
		for (auto &s : dynamicStates)
		{
			switch (s.type)
			{
			case DynamicStateType::viewport:
				vkDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
				break;
			case DynamicStateType::scissor:
				vkDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
				break;
			}
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

		_currentUboBinding = 0;

		for (auto s : stages)
		{
			if (!s) continue;

			s->create();

			VkPipelineShaderStageCreateInfo i = {};
			i.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			i.pName = "main";
			i.stage = (VkShaderStageFlagBits)vkStage(s->type);
			i.module = s->module->v;
			vkStages.push_back(i);

			for (auto &d : s->module->descriptors)
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
			for (auto &p : s->module->pushConstantRanges)
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
			for (auto d : descriptorSetLayouts)
			{
				if (d->bindings.size() == vkDescriptors.size())
				{
					bool same = true;
					for (auto i = 0; i < d->bindings.size(); i++)
					{
						auto &binding = d->bindings[i];
						if (binding.binding != vkDescriptors[i].binding || binding.descriptorCount != vkDescriptors[i].descriptorCount ||
							binding.descriptorType != vkDescriptors[i].descriptorType || binding.stageFlags != vkDescriptors[i].stageFlags)
						{
							same = false;
							break;
						}
					}
					if (same)
					{
						descriptorSetLayout = d;
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				auto d = new DescriptorSetLayout;
				d->bindings.insert(d->bindings.begin(), vkDescriptors.begin(), vkDescriptors.end());

				VkDescriptorSetLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.bindingCount = vkDescriptors.size();
				info.pBindings = vkDescriptors.data();

				device.mtx.lock();
				auto res = vkCreateDescriptorSetLayout(device.v, &info, nullptr, &d->v);
				assert(res == VK_SUCCESS);
				device.mtx.unlock();

				descriptorSetLayouts.push_back(d);

				descriptorSetLayout = d;
			}
		}

		descriptorSet = new DescriptorSet(descriptorPool, descriptorSetLayout);

		{
			bool found = false;
			for (auto p : pipelineLayouts)
			{
				if (p->descriptorLayout == descriptorSetLayout->v && p->pushConstantRanges.size() == vkPushConstantRanges.size())
				{
					bool same = true;
					for (auto i = 0; i < p->pushConstantRanges.size(); i++)
					{
						auto &pc = p->pushConstantRanges[i];
						if (pc.offset != vkPushConstantRanges[i].offset || pc.size != vkPushConstantRanges[i].size ||
							pc.stageFlags != vkPushConstantRanges[i].stageFlags)
						{
							same = false;
							break;
						}
					}
					if (same)
					{
						pipelineLayout = p;
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				auto p = new PipelineLayout;
				p->descriptorLayout = descriptorSetLayout->v;
				p->pushConstantRanges.insert(p->pushConstantRanges.begin(), vkPushConstantRanges.begin(), vkPushConstantRanges.end());

				VkPipelineLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				info.setLayoutCount = 1;
				info.pSetLayouts = &descriptorSetLayout->v;
				info.pushConstantRangeCount = p->pushConstantRanges.size();
				info.pPushConstantRanges = p->pushConstantRanges.data();

				device.mtx.lock();
				auto res = vkCreatePipelineLayout(device.v, &info, nullptr, &p->v);
				assert(res == VK_SUCCESS);
				device.mtx.unlock();

				pipelineLayouts.push_back(p);

				pipelineLayout = p;
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
		dynamicState.dynamicStateCount = vkDynamicStates.size();
		dynamicState.pDynamicStates = vkDynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = pipelineLayout->v;
		pipelineInfo.stageCount = vkStages.size();
		pipelineInfo.pStages = vkStages.data();
		pipelineInfo.pVertexInputState = pVertexInputState;
		pipelineInfo.pInputAssemblyState = &assemblyState;
		pipelineInfo.pTessellationState = patch_control_points ? &tessState : nullptr;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pColorBlendState = &blendState;
		pipelineInfo.renderPass = renderPass->v;
		pipelineInfo.subpass = subpassIndex;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDynamicState = vkDynamicStates.size() ? &dynamicState : nullptr;

		device.mtx.lock();
		auto res = vkCreateGraphicsPipelines(device.v, 0, 1, &pipelineInfo, nullptr, &pipeline);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		linkDescriptors(descriptorSet);
	}

	Pipeline::~Pipeline()
	{
		pipelineLayout->refCount--;
		if (pipelineLayout->refCount == 0)
		{
			for (auto it = pipelineLayouts.begin(); it != pipelineLayouts.end(); it++)
			{
				if (*it == pipelineLayout)
				{
					pipelineLayouts.erase(it);
					delete pipelineLayout;
					break;
				}
			}
		}

		descriptorSetLayout->refCount--;
		if (descriptorSetLayout->refCount == 0)
		{
			for (auto it = descriptorSetLayouts.begin(); it != descriptorSetLayouts.end(); it++)
			{
				if (*it == descriptorSetLayout)
				{
					descriptorSetLayouts.erase(it);
					delete descriptorSetLayout;
					break;
				}
			}
		}

		delete descriptorSet;

		device.mtx.lock();
		vkDestroyPipeline(device.v, pipeline, nullptr);
		device.mtx.unlock();

		for (int i = 0; i < 5; i++)
			delete stages[i];
	}

	void Pipeline::linkDescriptors(DescriptorSet *set)
	{
		for (auto &link : links)
		{
			if (link.binding == -1)
			{
				bool found = false;
				for (auto s : stages)
				{
					if (found) break;
					if (!s) continue;
					for (auto &d : s->module->descriptors)
					{
						if (d.name == link.descriptor_name)
						{
							link.binding = d.binding;
							link.type = d.type;
							found = true;
							break;
						}
					}
				}
				if (!found)
					int cut = 1;
				//assert(found);
			}
			if (link.type == DescriptorType::null)
			{
				bool found = false;
				for (auto s : stages)
				{
					if (found) break;
					if (!s) continue;
					for (auto &d : s->module->descriptors)
					{
						if (d.binding == link.binding)
						{
							link.type = d.type;
							found = true;
							break;
						}
					}
				}
				if (!found)
					int cut = 1;
				//assert(found);
			}

			switch (link.type)
			{
			case DescriptorType::uniform_buffer:
			{
				if (!link.buffer)
					link.buffer = pResource->getBuffer(link.resource_name);
				if (link.buffer)
					set->setBuffer(link.binding, link.array_element, link.buffer);
				else
					printf("%s: unable to link resource %s (binding:%d, type:uniform buffer)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
				break;
			case DescriptorType::image_n_sampler:
			{
				if (!link.image)
					link.image = pResource->getImage(link.resource_name);
				if (link.image)
				{
					if (link.vkSampler == 0)
					{
						switch (link.sampler)
						{
						case SamplerType::none:
							break;
						case SamplerType::plain:
							link.vkSampler = plainSampler;
							break;
						case SamplerType::plain_unnormalized:
							link.vkSampler = plainUnnormalizedSampler;
							break;
						case SamplerType::color:
							link.vkSampler = colorSampler;
							break;
						case SamplerType::color_border:
							link.vkSampler = colorBorderSampler;
							break;
						}
					}
					set->setImage(link.binding, link.array_element, link.image, link.vkSampler);
				}
				else
					printf("%s: unable to link resource %s (binding:%d, type:combined image sampler)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
				break;
			}
		}
	}

	int Pipeline::descriptorPosition(const std::string &name)
	{
		for (auto s : stages)
		{
			if (!s) continue;

			for (auto &d : s->module->descriptors)
			{
				if (d.name == name)
					return d.binding;
			}
		}
		return -1;
	}

#define NOMINMAX
#include <FreeImage.h>

	ImageData::~ImageData()
	{
		delete[]v;
	}

	VkFormat ImageData::getVkFormat(bool sRGB)
	{
		switch (channel)
		{
		case 1:
			switch (byte_per_pixel)
			{
			case 1:
				return VK_FORMAT_R8_UNORM;
			case 2:
				return VK_FORMAT_R16_UNORM;
			}
			break;
		case 4:
			switch (byte_per_pixel)
			{
			case 4:
				if (sRGB)
					return VK_FORMAT_R8G8B8A8_SRGB;
				else  
					return VK_FORMAT_R8G8B8A8_UNORM;
				break;
			}
		}

		return VK_FORMAT_UNDEFINED;
	}

	ImageData *createImageData(const std::string &filename)
	{
		auto fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename.c_str());
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename.c_str());
		if (fif == FIF_UNKNOWN) 
			return nullptr;
		auto dib = FreeImage_Load(fif, filename.c_str());
		if (!dib) 
			return nullptr;
		if (fif == FREE_IMAGE_FORMAT::FIF_JPEG || fif == FREE_IMAGE_FORMAT::FIF_TARGA || fif == FREE_IMAGE_FORMAT::FIF_PNG)
			FreeImage_FlipVertical(dib);

		auto pData = new ImageData;
		auto colorType = FreeImage_GetColorType(dib);
		pData->fif = fif;
		switch (colorType)
		{
		case FIC_MINISBLACK: case FIC_MINISWHITE:
			pData->channel = 1;
			break;
		case FIC_RGB:
		{
			auto newDib = FreeImage_ConvertTo32Bits(dib);
			FreeImage_Unload(dib);
			dib = newDib;
			pData->channel = 4;
		}
			break;
		case FIC_RGBALPHA:
			pData->channel = 4;
			break;
		}
		pData->cx = FreeImage_GetWidth(dib);
		pData->cy = FreeImage_GetHeight(dib);
		pData->byte_per_pixel = FreeImage_GetBPP(dib) / 8;
		pData->pitch = FreeImage_GetPitch(dib);
		pData->size = pData->pitch * pData->cy;
		pData->v = new unsigned char[pData->size];
		memcpy(pData->v, FreeImage_GetBits(dib), pData->size);
		FreeImage_Unload(dib);

		if (pData->channel == 4)
		{
			if (fif == FREE_IMAGE_FORMAT::FIF_BMP ||
				fif == FREE_IMAGE_FORMAT::FIF_TARGA ||
				fif == FREE_IMAGE_FORMAT::FIF_JPEG ||
				fif == FREE_IMAGE_FORMAT::FIF_PNG)
			{
				for (int y = 0; y < pData->cy; y++)
				{
					for (int x = 0; x < pData->cx; x++)
					{
						std::swap(pData->v[y * pData->pitch + x * pData->channel + 0],
							pData->v[y * pData->pitch + x * pData->channel + 2]);
					}
				}
			}
		}

		return pData;
	}

	Image *createImage(const std::string &filename, bool sRGB, bool saveData)
	{
		auto d = createImageData(filename);
		assert(d);

		auto i = new Image(d->cx, d->cy, d->getVkFormat(sRGB), VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, d->v, d->size);
		i->filename = filename;
		i->bytePerPixel = d->byte_per_pixel;
		i->pitch = d->pitch;
		i->sRGB = sRGB;

		if (saveData)
		{
			i->data = d->v;
			d->v = nullptr;
		}
		delete d;

		return i;
	}
}
