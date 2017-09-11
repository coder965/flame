#include <assert.h>

#include "command_buffer.h"
#include "buffer.h"
#include "image.h"
#include "renderpass.h"
#include "framebuffer.h"
#include "../model/model.h"

namespace tke
{
	CommandBuffer::CommandBuffer(VkCommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = level;
		info.commandPool = commandPool->v;
		info.commandBufferCount = 1;

		device.mtx.lock();
		auto res = vkAllocateCommandBuffers(device.v, &info, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	CommandBuffer::~CommandBuffer()
	{
		device.mtx.lock();
		vkFreeCommandBuffers(device.v, commandPool->v, 1, &v);
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

	void CommandBuffer::imageBarrier(VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout, Image *image, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = srcAccessFlags;
		barrier.dstAccessMask = dstAccessFlags;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image->v;
		if (image->type == Image::eColor || image->type == Image::eSwapchain)
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		else
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrier.subresourceRange.baseMipLevel = baseLevel;
		barrier.subresourceRange.levelCount = levelCount;
		barrier.subresourceRange.baseArrayLayer = baseLayer;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vkCmdPipelineBarrier(v, srcStageFlags, dstStageFlags, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
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

	void CommandBuffer::bindDescriptorSet(VkDescriptorSet *sets, int index, int count)
	{
		vkCmdBindDescriptorSets(v, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipelineLayout->v, index, count, sets, 0, nullptr);
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

	void CommandBuffer::drawModel(Model *m, int gIndex, int instanceCount, int firstInstance)
	{
		if (gIndex == -1)
			drawIndex(m->indices.size(), m->indiceBase, m->vertexBase, instanceCount, firstInstance);
		else
			drawIndex(m->geometries[gIndex]->indiceCount, m->indiceBase + m->geometries[gIndex]->indiceBase, m->vertexBase, instanceCount, firstInstance);
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

	CommandBuffer *begineOnceCommandBuffer()
	{
		auto cb = new CommandBuffer;
		cb->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return cb;
	}

	void endOnceCommandBuffer(CommandBuffer *cb)
	{
		cb->end();

		graphicsQueue.submit(1, &cb->v);
		graphicsQueue.waitIdle();

		delete cb;
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, size_t srcOffset, size_t dstOffset)
	{
		auto cb = begineOnceCommandBuffer();
		VkBufferCopy region = { srcOffset, dstOffset, size };
		vkCmdCopyBuffer(cb->v, srcBuffer, dstBuffer, 1, &region);
		endOnceCommandBuffer(cb);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t count, VkBufferCopy *ranges)
	{
		auto cb = begineOnceCommandBuffer();
		vkCmdCopyBuffer(cb->v, srcBuffer, dstBuffer, count, ranges);
		endOnceCommandBuffer(cb);
	}

	void updateBuffer(void *data, size_t size, Buffer *stagingBuffer, VkBuffer &Buffer)
	{
		void* map = stagingBuffer->map(0, size);
		memcpy(map, data, size);
		stagingBuffer->unmap();

		copyBuffer(stagingBuffer->v, Buffer, size);
	}

	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
	{
		auto cb = begineOnceCommandBuffer();
		VkImageCopy region = {};
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.layerCount = 1;
		region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.layerCount = 1;
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;
		vkCmdCopyImage(cb->v, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		endOnceCommandBuffer(cb);
	}

	thread_local CommandPool *commandPool = nullptr;
}