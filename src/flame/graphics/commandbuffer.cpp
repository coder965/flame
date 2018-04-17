#include "device_private.h"
#include "commandbuffer_private.h"
#include "renderpass_private.h"
#include "framebuffer_private.h"
#include "pipeline_private.h"
#include "descriptor_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace graphics
	{
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
			info.renderArea.extent.width = f->_priv->cx;
			info.renderArea.extent.height = f->_priv->cy;
			info.clearValueCount = r->_priv->clear_values.size();
			info.pClearValues = r->_priv->clear_values.data();

			vkCmdBeginRenderPass(_priv->v, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void Commandbuffer::end_renderpass()
		{
			vkCmdEndRenderPass(_priv->v);
		}

		void Commandbuffer::bind_pipeline(Pipeline *p)
		{
			if (_priv->current_pipeline == p)
				return;
			_priv->current_pipeline = p;
			vkCmdBindPipeline(_priv->v, VK_PIPELINE_BIND_POINT_GRAPHICS, p->_priv->v);
		}

		void Commandbuffer::bind_descriptorset(Descriptorset *s)
		{
			vkCmdBindDescriptorSets(_priv->v, VK_PIPELINE_BIND_POINT_GRAPHICS, _priv->current_pipeline->_priv->pipelinelayout->_priv->v, 0, 1, &s->_priv->v, 0, nullptr);
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

		void Commandbuffer::draw(int count)
		{
			vkCmdDraw(_priv->v, count, 1, 0, 0);
		}

		void Commandbuffer::draw_indexed(int count, int first_index)
		{
			vkCmdDrawIndexed(_priv->v, count, 1, first_index, 0, 0);
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
