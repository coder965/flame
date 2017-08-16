#pragma once

#include "vulkan.h"

namespace tke
{
	struct Buffer
	{
		size_t size = 0;
		VkBuffer v = 0;
		VkDeviceMemory memory = 0;

		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memoryProperty;

		Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		~Buffer();
		void recreate(size_t _size, void *data = nullptr);
		void update(void *data, StagingBuffer *stagingBuffer, size_t size = 0);
		void *map(size_t offset, size_t _size);
		void unmap();
	};

	struct StagingBuffer : Buffer
	{
		StagingBuffer(size_t _size);
	};

	struct UniformBuffer : Buffer
	{
		UniformBuffer(size_t _size);
	};

	struct VertexBuffer : Buffer
	{
		VertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndexBuffer : Buffer
	{
		IndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct OnceVertexBuffer : Buffer // data use once per frame
	{
		OnceVertexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct OnceIndexBuffer : Buffer // data use once per frame
	{
		OnceIndexBuffer(size_t _size = 16, void *data = nullptr);
	};

	struct IndirectVertexBuffer : Buffer
	{
		IndirectVertexBuffer(size_t _size = sizeof VkDrawIndirectCommand);
	};

	struct IndirectIndexBuffer : Buffer
	{
		IndirectIndexBuffer(size_t _size = sizeof VkDrawIndexedIndirectCommand);
	};
}
