#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	enum BufferType
	{
		BufferTypeStaging,
		BufferTypeUniform,
		BufferTypeVertex,
		BufferTypeIndex,
		BufferTypeImmediateVertex,
		BufferTypeImmediateIndex,
		BufferTypeIndirectVertex,
		BufferTypeIndirectIndex
	};

	struct Buffer
	{
		BufferType type;

		int size;
		VkBuffer v;
		VkDeviceMemory memory;

		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memory_property;

		VkDescriptorBufferInfo info;

		Buffer(BufferType _type, int _size);
		~Buffer();
		void create();
		void resize(int new_size);
		void *map(int offset = 0, int _size = 0);
		void unmap();
		void copy_to(Buffer *dst, int size, int src_offset = 0, int dst_offset = 0);
		void copy_to(Buffer *dst, int range_count, VkBufferCopy *ranges);
		void update(void *data, int _size = 0, Buffer *staging_buffer = nullptr);
	};

	extern Buffer *defalut_staging_buffer;

	void init_buffer();
}
