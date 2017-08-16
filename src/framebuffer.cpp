#include <assert.h>

#include "framebuffer.h"
#include "image.h"
#include "renderpass.h"

namespace tke
{
	Framebuffer::~Framebuffer()
	{
		device.mtx.lock();
		vkDestroyFramebuffer(device.v, v, nullptr);
		device.mtx.unlock();
	}

	std::vector<Framebuffer*> framebuffers;

	Framebuffer *getFramebuffer(Image *i, RenderPass *renderPass, int level)
	{
		auto view = i->getView(0, level);
		return getFramebuffer(i->levels[level].cx, i->levels[level].cy, renderPass, 1, &view);
	}

	Framebuffer *getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views)
	{
		for (auto f : framebuffers)
		{
			if (f->views.size() == viewCount)
			{
				bool same = true;
				for (auto i = 0; i < f->views.size(); i++)
				{
					if (f->views[i] != views[i])
					{
						same = false;
						break;
					}
				}
				if (same)
				{
					f->refCount++;
					return f;
				}
			}
		}

		auto f = new Framebuffer;
		f->cx = cx;
		f->cy = cy;
		for (int i = 0; i < viewCount; i++)
			f->views.push_back(views[i]);

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = cx;
		info.height = cy;
		info.layers = 1;
		info.renderPass = renderPass->v;
		info.attachmentCount = viewCount;
		info.pAttachments = views;

		device.mtx.lock();
		auto res = vkCreateFramebuffer(device.v, &info, nullptr, &f->v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		framebuffers.push_back(f);
		return f;
	}

	void releaseFramebuffer(Framebuffer *f)
	{
		f->refCount--;
		if (f->refCount == 0)
		{
			for (auto it = framebuffers.begin(); it != framebuffers.end(); it++)
			{
				if (*it == f)
				{
					framebuffers.erase(it);
					delete f;
					return;
				}
			}
		}
	}

}
