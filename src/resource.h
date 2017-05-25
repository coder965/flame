#ifndef __TKE_RESOURCE__
#define __TKE_RESOURCE__

#include <string>
#include <map>

#include "vk.h"

namespace tke
{
	struct Buffer;
	struct Image;
	struct Model;
	struct Pipeline;
	struct ResourceBank
	{
		std::map<std::string, Buffer*> bufferResources;
		std::map<std::string, Image*> imageResources;
		std::map<std::string, Model*> modelResources;
		std::map<std::string, Pipeline*> pipelineResources;
		std::map<std::string, VkCommandBuffer> cmdResources;

		void setBuffer(Buffer *p, const std::string &str);
		void setImage(Image *p, const std::string &str);
		void setModel(Model *p, const std::string &str);
		void setPipeline(Pipeline *p, const std::string &str);
		void setCmd(VkCommandBuffer p, const std::string &str);
		Buffer *getBuffer(const std::string &str);
		Image *getImage(const std::string &str);
		Model *getModel(const std::string &str);
		Pipeline *getPipeline(const std::string &str);
		VkCommandBuffer getCmd(const std::string &str);
	};

	extern ResourceBank globalResource;
}

#endif