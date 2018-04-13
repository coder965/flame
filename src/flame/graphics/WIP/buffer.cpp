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

	void Buffer::resize(int new_size)
	{
		if (size == new_size)
			return;

		vkFreeMemory(vk_device, memory, nullptr);
		vkDestroyBuffer(vk_device, v, nullptr);
		size = new_size;
		create();
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
