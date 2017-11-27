#pragma once

#include "vulkan.h"

namespace tke
{
	struct StagingBuffer;
	struct Buffer
	{
		size_t size = 0;
		VkBuffer v = 0;
		VkDeviceMemory memory = 0;

		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memoryProperty;

		VkDescriptorBufferInfo info;

		// must call in main thread
		Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		// must call in main thread
		~Buffer();
		void recreate(size_t _size, void *data = nullptr);
		void update(void *data, StagingBuffer *stagingBuffer, size_t size = 0);
		void *map(size_t offset, size_t _size);
		void unmap();
	};

	struct StagingBuffer : Buffer
	{
		// must call in main thread
		StagingBuffer(size_t _size);
	};

	struct UniformBuffer : Buffer
	{
		// must call in main thread
		UniformBuffer(size_t _size);
	};

	struct VertexBuffer : Buffer
	{
		// must call in main thread
		VertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndexBuffer : Buffer
	{
		// must call in main thread
		IndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct OnceVertexBuffer : Buffer // data use once per frame
	{
		// must call in main thread
		OnceVertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct OnceIndexBuffer : Buffer // data use once per frame
	{
		// must call in main thread
		OnceIndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndirectVertexBuffer : Buffer
	{
		// must call in main thread
		IndirectVertexBuffer(size_t _size = sizeof VkDrawIndirectCommand);
	};

	struct IndirectIndexBuffer : Buffer
	{
		// must call in main thread
		IndirectIndexBuffer(size_t _size = sizeof VkDrawIndexedIndirectCommand);
	};
}
