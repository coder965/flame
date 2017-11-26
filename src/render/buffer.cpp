#include <assert.h>

#include "buffer.h"
#include "command_buffer.h"

namespace tke
{
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
		allocInfo.memoryTypeIndex = findVkMemoryType(memRequirements.memoryTypeBits, p->memoryProperty);

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
		copyBuffer(stagingBuffer.v, p->v, p->size);
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

		info.offset = 0;
		info.buffer = v;
		info.range = size;
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
		updateBuffer(data, _size, stagingBuffer, v);
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
}
