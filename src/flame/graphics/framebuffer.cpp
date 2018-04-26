#include "device_private.h"
#include "framebuffer_private.h"
#include "renderpass_private.h"
#include "texture_private.h"
#include "swapchain_private.h"

namespace flame
{
	namespace graphics
	{
		void Framebuffer::set_view(int index, Textureview *v)
		{
			assert(index < _priv->views.size());

			_priv->views[index] = v->_priv->v;
		}

		void Framebuffer::set_view_swapchain(int index, Swapchain *s, int image_index)
		{
			assert(index < _priv->views.size());

			_priv->views[index] = s->_priv->image_views[image_index];
		}

		void Framebuffer::build()
		{
			VkFramebufferCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.width = cx;
			info.height = cy;
			info.layers = 1;
			info.renderPass = _priv->renderpass->_priv->v;
			info.attachmentCount = _priv->views.size();
			info.pAttachments = _priv->views.data();

			release();

			vk_chk_res(vkCreateFramebuffer(_priv->d->_priv->device , &info, nullptr, &_priv->v));

			valid = true;
		}

		void Framebuffer::release()
		{
			if (_priv->v)
			{
				vkDestroyFramebuffer(_priv->d->_priv->device, _priv->v, nullptr);
				valid = false;
			}
		}

		Framebuffer *create_framebuffer(Device *d, int cx, int cy, Renderpass *r)
		{
			auto f = new Framebuffer;
			f->cx = cx;
			f->cy = cy;
			f->valid = false;

			f->_priv = new FramebufferPrivate;
			f->_priv->d = d;
			f->_priv->renderpass = r;
			f->_priv->views.resize(r->_priv->attachments.size());
			f->_priv->v = 0;

			return f;
		}

		void destroy_framebuffer(Device *d, Framebuffer *f)
		{
			assert(d == f->_priv->d);

			f->release();

			delete f->_priv;
			delete f;
		}
	}
}

