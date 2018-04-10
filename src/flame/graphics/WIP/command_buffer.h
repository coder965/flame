#pragma once

#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct Pipeline;
	struct RenderPass;
	struct Framebuffer;
	struct Buffer;
	struct Texture;
	struct Model;
	struct RenderPass;

	struct CommandBuffer
	{
		VkCommandBuffer v;
		Pipeline *currentPipeline = nullptr;

		// must call in main thread
		CommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		// must call in main thread
		~CommandBuffer();
		void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, VkCommandBufferInheritanceInfo *pInheritance = nullptr);
		void end();
		void begin_renderpass(RenderPass *renderPass, Framebuffer *fb, VkClearValue *pClearValue = nullptr);
		void next_subpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
		void end_renderpass();
		void set_viewport_and_scissor(int cx, int cy);
		void set_scissor(int x, int y, int cx, int cy);
		void bind_vertex_buffer(Buffer *b);
		void bind_vertex_buffer(VkBuffer *b, int count, VkDeviceSize *offsets);
		void bind_vertex_buffer2(Buffer *b0, Buffer *b1);
		void bind_index_buffer(Buffer *b, VkIndexType type = VK_INDEX_TYPE_UINT32);
		void bind_pipeline(Pipeline *p);
		void bind_descriptor_set();
		void bind_descriptor_set(VkDescriptorSet *sets, int index = 0, int count = 1);
		void exec_secondary_cmd(VkCommandBuffer cmd);
		void push_constant(VkShaderStageFlags stage, int offset, int size, void *src);
		void draw(int vertexCount, int firstVertex = 0, int instanceCount = 1, int firstInstance = 0);
		void draw_index(int indexCount, int firstIndex = 0, int vertexOffset = 0, int instanceCount = 1, int firstInstance = 0);
		void draw_model(Model *m, int gIndex = -1, int instanceCount = 1, int firstInstance = 0);
		void draw_indirect(Buffer *b, int count, int offset = 0);
		void draw_indirect_index(Buffer *b, int count, int offset = 0);
		void wait_events(size_t count, VkEvent *e);
		void set_event(VkEvent e);
		void reset_event(VkEvent e);
	};

	CommandBuffer *begin_once_command_buffer();
	void end_once_command_buffer(CommandBuffer *cb);

	extern CommandPool *command_pool;
}
