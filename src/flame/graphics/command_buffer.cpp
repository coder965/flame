#include <assert.h>

#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/pipeline.h>
#include <flame/model/model.h>
#include <flame/graphics/command_buffer.h>

namespace tke
{
	CommandBuffer::CommandBuffer(VkCommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = level;
		info.commandPool = command_pool->v;
		info.commandBufferCount = 1;

		auto res = vkAllocateCommandBuffers(vk_device.v, &info, &v);
		assert(res == VK_SUCCESS);
	}

	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(vk_device.v, command_pool->v, 1, &v);
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

	void CommandBuffer::begin_renderpass(RenderPass *renderPass, Framebuffer *fb, VkClearValue *pClearValue)
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

	void CommandBuffer::next_subpass(VkSubpassContents contents)
	{
		vkCmdNextSubpass(v, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::end_renderpass()
	{
		vkCmdEndRenderPass(v);
	}

	void CommandBuffer::set_viewport_and_scissor(int cx, int cy)
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

	void CommandBuffer::set_scissor(int x, int y, int cx, int cy)
	{
		VkRect2D scissor;
		scissor.extent.width = cx;
		scissor.extent.height = cy;
		scissor.offset.x = x;
		scissor.offset.y = y;
		vkCmdSetScissor(v, 0, 1, &scissor);
	}

	void CommandBuffer::bind_vertex_buffer(Buffer *b)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(v, 0, 1, &b->v, offsets);
	}

	void CommandBuffer::bind_vertex_buffer(VkBuffer *b, int count, VkDeviceSize *offsets)
	{
		vkCmdBindVertexBuffers(v, 0, count, b, offsets);
	}

	void CommandBuffer::bind_vertex_buffer2(Buffer *b0, Buffer *b1)
	{
		VkBuffer buffers[] = {
			b0->v,
			b1->v
		};
		VkDeviceSize offsets[] = {
			0,
			0
		};
		vkCmdBindVertexBuffers(v, 0, 2, buffers, offsets);
	}

	void CommandBuffer::bind_index_buffer(Buffer *b, VkIndexType type)
	{
		vkCmdBindIndexBuffer(v, b->v, 0, type);
	}

	void CommandBuffer::bind_pipeline(Pipeline *p)
	{
		if (currentPipeline == p)
			return;
		currentPipeline = p;
		vkCmdBindPipeline(v, VK_PIPELINE_BIND_POINT_GRAPHICS, p->v);
	}

	void CommandBuffer::bind_descriptor_set()
	{
		vkCmdBindDescriptorSets(v, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipeline_layout->v, 0, 1, &currentPipeline->descriptor_set->v, 0, nullptr);
	}

	void CommandBuffer::bind_descriptor_set(VkDescriptorSet *sets, int index, int count)
	{
		vkCmdBindDescriptorSets(v, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipeline_layout->v, index, count, sets, 0, nullptr);
	}

	void CommandBuffer::exec_secondary_cmd(VkCommandBuffer cmd)
	{
		vkCmdExecuteCommands(v, 1, &cmd);
	}

	void CommandBuffer::push_constant(VkShaderStageFlags stage, int offset, int size, void *src)
	{
		vkCmdPushConstants(v, currentPipeline->pipeline_layout->v, stage, offset, size, src);
	}

	void CommandBuffer::draw(int vertexCount, int firstVertex, int instanceCount, int firstInstance)
	{
		vkCmdDraw(v, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::draw_index(int indexCount, int firstIndex, int vertexOffset, int instanceCount, int firstInstance)
	{
		vkCmdDrawIndexed(v, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::draw_model(Model *m, int gIndex, int instanceCount, int firstInstance)
	{
		if (gIndex == -1)
			draw_index(m->indices.size(), m->indice_base, m->vertex_base, instanceCount, firstInstance);
		else
			draw_index(m->geometries[gIndex]->indiceCount, m->indice_base + m->geometries[gIndex]->indiceBase, m->vertex_base, instanceCount, firstInstance);
	}

	void CommandBuffer::draw_indirect(Buffer *b, int count, int offset)
	{
		vkCmdDrawIndirect(v, b->v, offset * sizeof VkDrawIndirectCommand, count, sizeof VkDrawIndirectCommand);
	}

	void CommandBuffer::draw_indirect_index(Buffer *b, int count, int offset)
	{
		vkCmdDrawIndexedIndirect(v, b->v, offset * sizeof VkDrawIndexedIndirectCommand, count, sizeof VkDrawIndexedIndirectCommand);
	}

	void CommandBuffer::wait_events(size_t count, VkEvent *e)
	{
		vkCmdWaitEvents(v, count, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
	}

	void CommandBuffer::set_event(VkEvent e)
	{
		vkCmdSetEvent(v, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	void CommandBuffer::reset_event(VkEvent e)
	{
		vkCmdResetEvent(v, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	CommandPool::CommandPool()
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = 0;
		auto res = vkCreateCommandPool(vk_device.v, &info, nullptr, &v);
		assert(res == VK_SUCCESS);
	}

	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(vk_device.v, v, nullptr);
	}

	CommandBuffer *begin_once_command_buffer()
	{
		auto cb = new CommandBuffer;
		cb->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return cb;
	}

	void end_once_command_buffer(CommandBuffer *cb)
	{
		cb->end();

		vk_graphics_queue.submit(1, &cb->v);
		vk_graphics_queue.waitIdle();

		delete cb;
	}

	CommandPool *command_pool = nullptr;
}
