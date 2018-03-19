#include <assert.h>

#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/command_buffer.h>

namespace flame
{
	Buffer::Buffer(BufferType _type, int _size) :
		type(_type),
		size(_size),
		mapped(nullptr)
	{
		if (size < 1)
			size = 1;

		create();
	}

	Buffer::~Buffer()
	{
		vkFreeMemory(vk_device, memory, nullptr);
		vkDestroyBuffer(vk_device, v, nullptr);
	}

	void Buffer::create()
	{
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memory_property;

		switch (type)
		{
			case BufferTypeStaging:
				usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				break;
			case BufferTypeUniform:
				usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			case BufferTypeVertex:
				usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			case BufferTypeIndex:
				usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			case BufferTypeImmediateVertex:
				usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
				break;
			case BufferTypeImmediateIndex:
				usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
				break;
			case BufferTypeIndirectVertex:
				usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			case BufferTypeIndirectIndex:
				usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
		}

		VkBufferCreateInfo buffer_info;
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.flags = 0;
		buffer_info.pNext = nullptr;
		buffer_info.queueFamilyIndexCount = 0;
		buffer_info.pQueueFamilyIndices = nullptr;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buffer_info.size = size;
		buffer_info.usage = usage;

		auto res = vkCreateBuffer(vk_device, &buffer_info, nullptr, &v);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(vk_device, v, &mem_requirements);

		assert(size <= mem_requirements.size);

		VkMemoryAllocateInfo alloc_info;
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.pNext = nullptr;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = vk_find_memory_type(mem_requirements.memoryTypeBits, memory_property);

		vk_chk_res(vkAllocateMemory(vk_device, &alloc_info, nullptr, &memory));

		vk_chk_res(vkBindBufferMemory(vk_device, v, memory, 0));

		info.offset = 0;
		info.buffer = v;
		info.range = size;
	}

	void Buffer::resize(int new_size)
	{
		if (size == new_size)
			return;

		vkFreeMemory(vk_device, memory, nullptr);
		vkDestroyBuffer(vk_device, v, nullptr);
		size = new_size;
		create();
	}

	void Buffer::map(int offset, int _size)
	{
		if (_size == 0)
			_size = size;
		vk_chk_res(vkMapMemory(vk_device, memory, offset, _size, 0, &mapped));
	}

	void Buffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(vk_device, memory);
			mapped = nullptr;
		}
	}

	void Buffer::flush()
	{
		VkMappedMemoryRange range;
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = nullptr;
		range.memory = memory;
		range.offset = 0;
		range.size = size;
		vk_chk_res(vkFlushMappedMemoryRanges(vk_device, 1, &range));
	}

	void Buffer::copy_to(Buffer *dst, int size, int src_offset, int dst_offset)
	{
		auto cb = begin_once_command_buffer();
		VkBufferCopy region = { src_offset, dst_offset, size };
		vkCmdCopyBuffer(cb->v, v, dst->v, 1, &region);
		end_once_command_buffer(cb);
	}

	void Buffer::copy_to(Buffer *dst, int range_count, VkBufferCopy *ranges)
	{
		if (range_count <= 0)
			return;

		auto cb = begin_once_command_buffer();
		vkCmdCopyBuffer(cb->v, v, dst->v, range_count, ranges);
		end_once_command_buffer(cb);
	}

	void Buffer::update(void *data, int _size, Buffer *staging_buffer)
	{
		if (_size == 0) 
			_size = size;
		if (staging_buffer == nullptr)
			staging_buffer = defalut_staging_buffer;
		if (_size > staging_buffer->size)
			staging_buffer->resize(_size);

		staging_buffer->map(0, _size);
		memcpy(staging_buffer->mapped, data, _size);
		staging_buffer->unmap();
		staging_buffer->copy_to(this, _size);
	}

	Buffer *defalut_staging_buffer = nullptr;

	void init_buffer()
	{
		defalut_staging_buffer = new Buffer(BufferTypeStaging, 67108864);
	}
}
