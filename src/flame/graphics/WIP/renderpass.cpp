#include <assert.h>
#include <memory>

#include <flame/global.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/renderpass.h>

namespace flame
{
	bool operator==(const RenderPassInfo &lhs, const RenderPassInfo &rhs)
	{
		if (lhs.attachments.size() != rhs.attachments.size())
			return false;
		if (lhs.subpasses.size() != rhs.subpasses.size())
			return false;
		if (lhs.dependencies.size() != rhs.dependencies.size())
			return false;
		for (auto i = 0; i < lhs.attachments.size(); i++)
		{
			if (lhs.attachments[i].first != rhs.attachments[i].first)
				return false;
			if (lhs.attachments[i].second != rhs.attachments[i].second)
				return false;
		}
		for (auto i = 0; i < lhs.subpasses.size(); i++)
		{
			if (lhs.subpasses[i].first.size() != rhs.subpasses[i].first.size())
				return false;
			for (auto j = 0; j < lhs.subpasses[i].first.size(); j++)
			{
				if (lhs.subpasses[i].first[j] != rhs.subpasses[i].first[j])
					return false;
			}
			if (lhs.subpasses[i].second != rhs.subpasses[i].second)
				return false;
		}
		for (auto i = 0; i < lhs.dependencies.size(); i++)
		{
			if (lhs.dependencies[i].first != rhs.dependencies[i].first)
				return false;
			if (lhs.dependencies[i].second != rhs.dependencies[i].second)
				return false;
		}
		return true;
	}

	static std::vector<std::weak_ptr<RenderPass>> _renderpasses;

	std::shared_ptr<RenderPass> get_renderpass(const RenderPassInfo &_info)
	{
		for (auto it = _renderpasses.begin(); it != _renderpasses.end(); )
		{
			auto s = it->lock();
			if (s)
			{
				if (s->info == _info)
					return s;
				it++;
			}
			else
				it = _renderpasses.erase(it);
		}
		auto s = std::make_shared<RenderPass>(_info);
		_renderpasses.push_back(s);
		return s;
	}
}
