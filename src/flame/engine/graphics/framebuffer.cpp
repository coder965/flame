#include <assert.h>

#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/renderpass.h>

namespace flame
{
	Framebuffer::~Framebuffer()
	{
		vkDestroyFramebuffer(vk_device, v, nullptr);
	}

	static std::vector<std::weak_ptr<Framebuffer>> _framebuffers;

	std::shared_ptr<Framebuffer> get_framebuffer(Texture *i, RenderPass *renderpass, int level)
	{
		auto view = i->get_view(VK_IMAGE_VIEW_TYPE_2D, level);
		return get_framebuffer(i->levels[level]->cx, i->levels[level]->cy, renderpass, 1, &view);
	}

	std::shared_ptr<Framebuffer> get_framebuffer(int cx, int cy, RenderPass *renderpass, int view_count, VkImageView *views)
	{
		for (auto it = _framebuffers.begin(); it != _framebuffers.end(); )
		{
			auto f = it->lock();

			if (f)
			{
				if (f->views.size() == view_count)
				{
					bool same = true;
					for (auto i = 0; i < view_count; i++)
					{
						if (f->views[i] != views[i])
						{
							same = false;
							break;
						}
					}
					if (same)
						return f;
				}

				it++;
			}
			else
				it = _framebuffers.erase(it);
		}

		auto f = std::make_shared<Framebuffer>();
		f->cx = cx;
		f->cy = cy;
		for (int i = 0; i < view_count; i++)
			f->views.push_back(views[i]);

		VkFramebufferCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;
		info.width = cx;
		info.height = cy;
		info.layers = 1;
		info.renderPass = renderpass->v;
		info.attachmentCount = view_count;
		info.pAttachments = views;

		vk_chk_res(vkCreateFramebuffer(vk_device, &info, nullptr, &f->v));

		_framebuffers.push_back(f);
		return f;
	}
}
