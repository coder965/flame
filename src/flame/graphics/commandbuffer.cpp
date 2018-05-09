#include "device_private.h"
#include "commandbuffer_private.h"
#include "renderpass_private.h"
#include "framebuffer_private.h"
#include "pipeline_private.h"
#include "descriptor_private.h"
#include "buffer_private.h"
#include "texture_private.h"

namespace flame
{
	namespace graphics
	{
		inline VkImageAspectFlags Z(Format::Type t)
		{
			switch (t)
			{
				case Format::TypeColor:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case Format::TypeDepth:
					return VK_IMAGE_ASPECT_DEPTH_BIT;
				case Format::TypeDepthStencil:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		void Commandbuffer::begin(bool once)
		{
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT |
				(once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);
			info.pNext = nullptr;
			info.pInheritanceInfo = nullptr;
			vk_chk_res(vkBeginCommandBuffer(_priv->v, &info));
			_priv->current_pipeline = nullptr;
		}

		void Commandbuffer::begin_renderpass(Renderpass *r, Framebuffer *f)
		{
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = r->_priv->v;
			info.framebuffer = f->_priv->v;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			info.renderArea.extent.width = f->cx;
			info.renderArea.extent.height = f->cy;
			info.clearValueCount = r->_priv->clear_values.size();
			info.pClearValues = r->_priv->clear_values.data();

			vkCmdBeginRenderPass(_priv->v, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void Commandbuffer::end_renderpass()
		{
			vkCmdEndRenderPass(_priv->v);
		}

		void Commandbuffer::set_viewport(int x, int y, int width, int height)
		{
			VkViewport vp;
			vp.minDepth = 0.f;
			vp.maxDepth = 1.f;
			vp.x = x;
			vp.y = y;
			vp.width = width;
			vp.height = height;
			vkCmdSetViewport(_priv->v, 0, 1, &vp);
		}

		void Commandbuffer::set_scissor(int x, int y, int width, int height)
		{
			VkRect2D sc;
			sc.offset.x = x;
			sc.offset.y = y;
			sc.extent.width = width;
			sc.extent.height = height;
			vkCmdSetScissor(_priv->v, 0, 1, &sc);
		}

		void Commandbuffer::bind_pipeline(Pipeline *p)
		{
			if (_priv->current_pipeline == p)
				return;
			assert(p->type != PipelineNone);
			_priv->current_pipeline = p;
			vkCmdBindPipeline(_priv->v, Z(p->type), p->_priv->v);
		}

		void Commandbuffer::bind_descriptorset(Descriptorset *s)
		{
			vkCmdBindDescriptorSets(_priv->v, Z(_priv->current_pipeline->type), _priv->current_pipeline->_priv->pipelinelayout->_priv->v, 0, 1, &s->_priv->v, 0, nullptr);
		}

		void Commandbuffer::bind_vertexbuffer(Buffer *b)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(_priv->v, 0, 1, &b->_priv->v, &offset);
		}

		void Commandbuffer::bind_indexbuffer(Buffer *b, IndiceType t)
		{
			vkCmdBindIndexBuffer(_priv->v, b->_priv->v, 0, t == IndiceTypeUint ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void Commandbuffer::push_constant(int shader_stage, int offset, int size, void *data)
		{
			vkCmdPushConstants(_priv->v, _priv->current_pipeline->_priv->pipelinelayout->_priv->v, Z(ShaderType(shader_stage)), offset, size, data);
		}

		void Commandbuffer::draw(int count, int instance_count, int first_instance)
		{
			vkCmdDraw(_priv->v, count, instance_count, 0, first_instance);
		}

		void Commandbuffer::draw_indexed(int count, int first_index, int vertex_offset, int instance_count, int first_instance)
		{
			vkCmdDrawIndexed(_priv->v, count, instance_count, first_index, 0, first_instance);
		}

		void Commandbuffer::dispatch(int x, int y, int z)
		{
			vkCmdDispatch(_priv->v, x, y, z);
		}

		void Commandbuffer::copy_buffer(Buffer *src, Buffer *dst, int copy_count, BufferCopy *copies)
		{
			std::vector<VkBufferCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
			{
				vk_copies[i].srcOffset = copies[i].src_offset;
				vk_copies[i].dstOffset = copies[i].dst_offset;
				vk_copies[i].size = copies[i].size;
			}
			vkCmdCopyBuffer(_priv->v, src->_priv->v, dst->_priv->v, copy_count, vk_copies.data());
		}

		void Commandbuffer::change_texture_layout(Texture *t, TextureLayout from, TextureLayout to,
			int base_level, int level_count, int base_layer, int layer_count)
		{
			level_count = level_count == 0 ? t->level : level_count;
			layer_count = layer_count == 0 ? t->layer : layer_count;

			auto ft = t->format.get_type();

			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.oldLayout = Z(from, ft);
			barrier.newLayout = Z(to, ft);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = t->_priv->v;
			barrier.subresourceRange.aspectMask = Z(ft);
			barrier.subresourceRange.baseMipLevel = base_level;
			barrier.subresourceRange.levelCount = level_count;
			barrier.subresourceRange.baseArrayLayer = base_layer;
			barrier.subresourceRange.layerCount = layer_count;

			switch (barrier.oldLayout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:
					barrier.srcAccessMask = 0;
					break;
				case VK_IMAGE_LAYOUT_GENERAL:
					barrier.srcAccessMask = 0;
					break;
				case VK_IMAGE_LAYOUT_PREINITIALIZED:
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
					barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					break;
			}

			switch (barrier.newLayout)
			{
				case VK_IMAGE_LAYOUT_GENERAL:
					barrier.dstAccessMask = 0;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
					break;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					if (barrier.srcAccessMask == 0)
						barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
			}

			vkCmdPipelineBarrier(_priv->v, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		void Commandbuffer::copy_buffer_to_image(Buffer *src, Texture *dst, int copy_count, BufferImageCopy *copies)
		{
			auto aspect = Z(dst->format.get_type());

			std::vector<VkBufferImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
			{
				vk_copies[i] = {};
				vk_copies[i].bufferOffset = copies[i].buffer_offset;
				vk_copies[i].imageExtent.width = copies[i].image_width;
				vk_copies[i].imageExtent.height = copies[i].image_height;
				vk_copies[i].imageExtent.depth = 1;
				vk_copies[i].imageSubresource.aspectMask = aspect;
				vk_copies[i].imageSubresource.mipLevel = copies[i].image_level;
				vk_copies[i].imageSubresource.layerCount = 1;
			}
			vkCmdCopyBufferToImage(_priv->v, src->_priv->v, dst->_priv->v,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_count, vk_copies.data());
		}

		void Commandbuffer::end()
		{
			vk_chk_res(vkEndCommandBuffer(_priv->v));
		}

		Commandbuffer* Commandpool::create_commandbuffer(bool sub)
		{
			VkCommandBufferAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.level = !sub ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			info.commandPool = _priv->v;
			info.commandBufferCount = 1;

			auto c = new Commandbuffer;
			c->_priv = new CommandbufferPrivate;

			vk_chk_res(vkAllocateCommandBuffers(_priv->d->_priv->device, &info, &c->_priv->v));

			return c;
		}

		void Commandpool::destroy_commandbuffer(Commandbuffer *c)
		{
			vkFreeCommandBuffers(_priv->d->_priv->device, _priv->v, 1, &c->_priv->v);

			delete c->_priv;
			delete c;
		}

		Commandpool *create_commandpool(Device *d)
		{
			auto p = new Commandpool;
			p->_priv = new CommandpoolPrivate;
			p->_priv->d = d;

			VkCommandPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.pNext = nullptr;
			info.queueFamilyIndex = 0;

			vk_chk_res(vkCreateCommandPool(d->_priv->device, &info, nullptr, &p->_priv->v));

			return p;
		}

		void destroy_commandpool(Device *d, Commandpool *p)
		{
			assert(d == p->_priv->d);

			vkDestroyCommandPool(d->_priv->device, p->_priv->v, nullptr);

			delete p->_priv;
			delete p;
		}
	}
}
