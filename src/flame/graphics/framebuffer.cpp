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

#include "device_private.h"
#include "framebuffer_private.h"
#include "renderpass_private.h"
#include "texture_private.h"
#include "swapchain_private.h"

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_GRAPHICS_VULKAN)
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
			info.width = size.x;
			info.height = size.y;
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

		Framebuffer *create_framebuffer(Device *d, const Ivec2 &size, Renderpass *r)
		{
			auto f = new Framebuffer;
			f->size = size;
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
#endif
	}
}

