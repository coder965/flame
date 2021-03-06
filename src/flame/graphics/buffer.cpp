//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "buffer_private.h"
#include "device_private.h"

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_GRAPHICS_VULKAN)
		void Buffer::map(int offset, int _size)
		{
			if (_size == 0)
				_size = size;
			vk_chk_res(vkMapMemory(_priv->d->_priv->device, _priv->m, offset, _size, 0, &mapped));
		}

		void Buffer::unmap()
		{
			if (mapped)
			{
				vkUnmapMemory(_priv->d->_priv->device, _priv->m);
				mapped = nullptr;
			}
		}
		void Buffer::flush()
		{
			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = nullptr;
			range.memory = _priv->m;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			vk_chk_res(vkFlushMappedMemoryRanges(_priv->d->_priv->device, 1, &range));
		}
#else
		void Buffer::data(BufferUsage usage, int _size, void *data)
		{
			_priv->usage = usage;
			size = _size;
			auto target = buffer_target(usage);
			glBindBuffer(target, _priv->v);
			glBufferData(target, _size, data, GL_STREAM_DRAW);
		}
#endif

#if defined(FLAME_GRAPHICS_VULKAN)
		Buffer *create_buffer(Device *d, int size, int usage, int mem_prop)
		{
			auto b = new Buffer;
			b->size = size;
			b->mapped = nullptr;

			b->_priv = new BufferPrivate;
			b->_priv->usage = usage;
			b->_priv->mem_prop = mem_prop;
			b->_priv->d = d;

			VkBufferUsageFlags vk_usage = 0;
			if (usage & BufferUsageTransferSrc)
				vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (usage & BufferUsageTransferDst)
				vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (usage & BufferUsageUniformBuffer)
				vk_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (usage & BufferUsageStorageBuffer)
				vk_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (usage & BufferUsageVertexBuffer)
				vk_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if (usage & BufferUsageIndexBuffer)
				vk_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (usage & BufferUsageIndirectBuffer)
				vk_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.queueFamilyIndexCount = 0;
			buffer_info.pQueueFamilyIndices = nullptr;
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.size = size;
			buffer_info.usage = vk_usage;

			auto res = vkCreateBuffer(d->_priv->device, &buffer_info, nullptr, &b->_priv->v);
			assert(res == VK_SUCCESS);

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(d->_priv->device, b->_priv->v, &mem_requirements);

			assert(size <= mem_requirements.size);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = d->_priv->find_memory_type(mem_requirements.memoryTypeBits, Z((MemProp)mem_prop));

			vk_chk_res(vkAllocateMemory(d->_priv->device, &alloc_info, nullptr, &b->_priv->m));

			vk_chk_res(vkBindBufferMemory(d->_priv->device, b->_priv->v, b->_priv->m, 0));

			return b;
	}
#else
		Buffer *create_buffer(Device *d)
		{
			auto b = new Buffer;
			b->size = 0;

			b->_priv = new BufferPrivate;
			b->_priv->usage = 0;

			glGenBuffers(1, &b->_priv->v);

			return b;
		}
#endif
		void destroy_buffer(Device *d, Buffer *b)
		{
#if defined(FLAME_GRAPHICS_VULKAN)
			if (b->mapped)
				b->unmap();

			vkFreeMemory(d->_priv->device, b->_priv->m, nullptr);
			vkDestroyBuffer(d->_priv->device, b->_priv->v, nullptr);
#else
			glDeleteBuffers(1, &b->_priv->v);
#endif

			delete b->_priv;
			delete b;
		}
	}
}

