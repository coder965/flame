#include <assert.h>

#include "buffer.h"
#include "command_buffer.h"

namespace tke
{
	static void buffer_create(Buffer *p)
	{
	}

	static void buffer_copy(Buffer *p, void *data)
	{
		StagingBuffer stagingBuffer(p->size);

		void* map = stagingBuffer.map(0, p->size);
		memcpy(map, data, p->size);
		stagingBuffer.unmap();
		stagingBuffer.copyTo(p, p->size);
	}

	Buffer::Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _memoryProperty)
	{
		size = _size;
		usage = _usage;
		memoryProperty = _memoryProperty;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto res = vkCreateBuffer(vk_device.v, &bufferInfo, nullptr, &v);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vk_device.v, v, &memRequirements);

		assert(size <= memRequirements.size);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_vk_memory_type(memRequirements.memoryTypeBits, memoryProperty);

		res = vkAllocateMemory(vk_device.v, &allocInfo, nullptr, &memory);
		assert(res == VK_SUCCESS);

		res = vkBindBufferMemory(vk_device.v, v, memory, 0);
		assert(res == VK_SUCCESS);

		info.buffer = v;
		info.range = size;

		info.offset = 0;
	}

	Buffer::~Buffer()
	{
		vkFreeMemory(vk_device.v, memory, nullptr);
		vkDestroyBuffer(vk_device.v, v, nullptr);
	}

	void *Buffer::map(size_t offset, size_t _size)
	{
		void *map;
		vk_device.mtx.lock();
		auto res = vkMapMemory(vk_device.v, memory, offset, _size, 0, &map);
		assert(res == VK_SUCCESS);
		vk_device.mtx.unlock();
		return map;
	}

	void Buffer::unmap()
	{
		vk_device.mtx.lock();
		vkUnmapMemory(vk_device.v, memory);
		vk_device.mtx.unlock();
	}

	void Buffer::copyTo(Buffer *dst, VkDeviceSize size, size_t srcOffset, size_t dstOffset)
	{
		auto cb = begineOnceCommandBuffer();
		VkBufferCopy region = {srcOffset, dstOffset, size};
		vkCmdCopyBuffer(cb->v, v, dst->v, 1, &region);
		endOnceCommandBuffer(cb);
	}

	void Buffer::copyTo(Buffer *dst, size_t count, VkBufferCopy *ranges)
	{
		if (count <= 0)
			return;
		auto cb = begineOnceCommandBuffer();
		vkCmdCopyBuffer(cb->v, v, dst->v, count, ranges);
		endOnceCommandBuffer(cb);
	}

	void Buffer::update(void *data, Buffer *stagingBuffer, size_t _size)
	{
		if (_size == 0) 
			_size = size;
		void* map = stagingBuffer->map(0, _size);
		memcpy(map, data, _size);
		stagingBuffer->unmap();
		stagingBuffer->copyTo(this, _size);
	}

	StagingBuffer::StagingBuffer(size_t _size)
		:Buffer(_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT/*dst usage for pick up*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
	}

	UniformBuffer::UniformBuffer(size_t _size)
		: Buffer(_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
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

	ImmediateVertexBuffer::ImmediateVertexBuffer(size_t _size, void *data)
		: Buffer(_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
		if (data)
			buffer_copy(this, data);
	}

	ImmediateIndexBuffer::ImmediateIndexBuffer(size_t _size, void *data)
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

	StagingBuffer *defalut_staging_buffer = nullptr;

	void init_buffer()
	{
		defalut_staging_buffer = new StagingBuffer(67108864);
	}
}
