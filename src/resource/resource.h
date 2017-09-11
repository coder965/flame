#pragma once

#include <map>

#include "../render/buffer.h"
#include "../render/image.h"

namespace tke
{
	struct Resource
	{
		Resource *parents;

		std::map<std::string, Buffer*> bufferResources;
		std::map<std::string, Image*> imageResources;

		void setBuffer(Buffer *p, const std::string &str);
		void setImage(Image *p, const std::string &str);

		Buffer *getBuffer(const std::string &str);
		Image *getImage(const std::string &str);

		Resource(Resource *_parent);
	};

	extern Resource globalResource;
}
