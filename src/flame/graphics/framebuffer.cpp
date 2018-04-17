#include "device_private.h"
#include "framebuffer_private.h"
#include "renderpass_private.h"
#include "texture_private.h"
#include "swapchain_private.h"

namespace flame
{
	namespace graphics
	{
		void Framebuffer::set_renderpass(Renderpass *r)
		{
			assert(_priv->v == 0);

			_priv->renderpass = r;
			_priv->views.resize(r->_priv->attachments.size());
		}

		void Framebuffer::set_size(int cx, int cy)
		{
			_priv->cx = cx == -1 ? _priv->d->resolution_x : cx;
			_priv->cy = cy == -1 ? _priv->d->resolution_y : cy;
		}

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
			info.width = _priv->cx;
			info.height = _priv->cy;
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

		Framebuffer *create_framebuffer(Device *d)
		{
			auto f = new Framebuffer;
			f->valid = false;

			f->_priv = new FramebufferPrivate;
			f->_priv->d = d;
			f->_priv->renderpass = nullptr;
			f->_priv->cx = 0;
			f->_priv->cy = 0;
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

