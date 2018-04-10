#include "commandbuffer_private.h"

namespace flame
{
	namespace graphics
	{
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

			vk_chk_res(vkAllocateCommandBuffers(_priv->g->_priv->device, &info, &c->_priv->v));
		}

		void Commandpool::destroy_commandbuffer(Commandbuffer *c)
		{
			vkFreeCommandBuffers(_priv->g->_priv->device, _priv->v, 1, &c->_priv->v);

			delete c->_priv;
			delete c;
		}

		Commandpool *create_commandpool(Graphics *g)
		{
			auto p = new Commandpool;
			p->_priv = new CommandpoolPrivate;
			p->_priv->g = g;

			VkCommandPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.pNext = nullptr;
			info.queueFamilyIndex = 0;

			vk_chk_res(vkCreateCommandPool(g->_priv->device, &info, nullptr, &p->_priv->v));

			return p;
		}

		void destroy_commandpool(Graphics *g, Commandpool *p)
		{
			assert(g == p->_priv->g);

			vkDestroyCommandPool(g->_priv->device, p->_priv->v, nullptr);

			delete p->_priv;
			delete p;
		}
	}
}
