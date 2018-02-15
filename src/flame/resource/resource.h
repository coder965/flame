#pragma once

#include <map>

namespace tke
{
	struct Buffer;
	struct Texture;

	struct Resource
	{
		Resource *parent;

		std::map<std::string, Buffer*> bufferResources;
		std::map<std::string, Texture*> imageResources;

		void setBuffer(Buffer *p, const std::string &str);
		void setImage(Texture *p, const std::string &str);

		Buffer *getBuffer(const std::string &str);
		Texture *getImage(const std::string &str);

		Resource(Resource *_parent);
	};

	extern Resource globalResource;
}
