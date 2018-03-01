#include <flame/resource/resource.h>

namespace flame
{
	Resource::Resource(Resource *_parent)
	{
		parent = _parent;
	}

	void Resource::setBuffer(Buffer *p, const std::string &str)
	{
		bufferResources[str] = p;
	}

	void Resource::setImage(Texture *p, const std::string &str)
	{
		imageResources[str] = p;
	}

	Buffer *Resource::getBuffer(const std::string &str)
	{
		auto it = bufferResources.find(str);
		if (it == bufferResources.end())
		{
			if (parent)
				return parent->getBuffer(str);
			else
				return nullptr;
		}
		return it->second;
	}

	Texture *Resource::getImage(const std::string &str)
	{
		auto it = imageResources.find(str);
		if (it == imageResources.end())
		{
			if (parent)
				return parent->getImage(str);
			else
				return nullptr;
		}
		return it->second;
	}

	Resource globalResource(nullptr);
}
