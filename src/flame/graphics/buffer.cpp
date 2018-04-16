#include "buffer_private.h"
#include "device_private.h"
#include "commandbuffer_private.h"

namespace flame
{
	namespace graphics
	{
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

		Buffer *create_buffer(Device *d, int size, int usage, int mem_prop)
		{
			auto b = new Buffer;
			b->size = size;
			b->_priv = new BufferPrivate;
			b->_priv->d = d;

			VkBufferUsageFlags vk_usage = 0;
			if (usage & BufferUsageTransferSrc)
				vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (usage & BufferUsageTransferDst)
				vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (usage & BufferUsageUniformBuffer)
				vk_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (usage & BufferUsageIndexBuffer)
				vk_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (usage & BufferUsageVertexBuffer)
				vk_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
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

		void destroy_buffer(Device *d, Buffer *b)
		{
			vkFreeMemory(d->_priv->device, b->_priv->m, nullptr);
			vkDestroyBuffer(d->_priv->device, b->_priv->v, nullptr);

			delete b->_priv;
			delete b;
		}

		void copy_buffer(Commandbuffer *c, Buffer *src, Buffer *dst, int src_offset, int dst_offset, int size)
		{
			VkBufferCopy region = {src_offset, dst_offset, size};
			vkCmdCopyBuffer(c->_priv->v, src->_priv->v, dst->_priv->v, 1, &region);
		}
	}
}

