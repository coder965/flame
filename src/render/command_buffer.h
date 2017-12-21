#pragma once

#include "vulkan.h"
#include "pipeline.h"

namespace tke
{
	struct Pipeline;
	struct RenderPass;
	struct Framebuffer;
	struct Buffer;
	struct VertexBuffer;
	struct ImmediateVertexBuffer;
	struct IndexBuffer;
	struct ImmediateIndexBuffer;
	struct Image;
	struct Model;
	struct IndirectVertexBuffer;
	struct IndirectIndexBuffer;
	struct CommandBuffer
	{
		VkCommandBuffer v;
		Pipeline *currentPipeline = nullptr;

		// must call in main thread
		CommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		// must call in main thread
		~CommandBuffer();
		void reset();
		void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, VkCommandBufferInheritanceInfo *pInheritance = nullptr);
		void end();
		void imageBarrier(VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout, Image *image, int baseLevel, int levelCount, int baseLayer, int layerCount);
		void beginRenderPass(RenderPass *renderPass, Framebuffer *fb, VkClearValue *pClearValue = nullptr);
		void nextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
		void endRenderPass();
		void setViewportAndScissor(int cx, int cy);
		void setScissor(int x, int y, int cx, int cy);
		void bindVertexBuffer(VertexBuffer *b);
		void bindVertexBuffer(VkBuffer *b, int count, VkDeviceSize *offsets);
		void bindVertexBuffer2(VertexBuffer *b0, VertexBuffer *b1);
		void bindVertexBuffer(ImmediateVertexBuffer *b);
		void bindIndexBuffer(IndexBuffer *b, VkIndexType type = VK_INDEX_TYPE_UINT32);
		void bindIndexBuffer(ImmediateIndexBuffer *b, VkIndexType type = VK_INDEX_TYPE_UINT32);
		void bindPipeline(Pipeline *p);
		void bindDescriptorSet();
		void bindDescriptorSet(VkDescriptorSet *sets, int index = 0, int count = 1);
		void execSecondaryCmd(VkCommandBuffer cmd);
		void pushConstant(VkShaderStageFlags stage, int offset, int size, void *src);
		void draw(int vertexCount, int firstVertex = 0, int instanceCount = 1, int firstInstance = 0);
		void drawIndex(int indexCount, int firstIndex = 0, int vertexOffset = 0, int instanceCount = 1, int firstInstance = 0);
		void drawModel(Model *m, int gIndex = -1, int instanceCount = 1, int firstInstance = 0);
		void drawIndirect(IndirectVertexBuffer *b, int count, int offset = 0);
		void drawIndirectIndex(IndirectIndexBuffer *b, int count, int offset = 0);
		void waitEvents(size_t count, VkEvent *e);
		void setEvent(VkEvent e);
		void resetEvent(VkEvent e);
	};

	struct CommandPool
	{
		VkCommandPool v;

		// must call in main thread
		CommandPool();
		// must call in main thread
		~CommandPool();
	};

	CommandBuffer *begineOnceCommandBuffer();
	void endOnceCommandBuffer(CommandBuffer *cb);
	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

	extern CommandPool *commandPool;
}
