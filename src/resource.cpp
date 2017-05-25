#include "resource.h"

namespace tke
{
	ResourceBank globalResource;

	void ResourceBank::setBuffer(Buffer *p, const std::string &str)
	{
		bufferResources[str] = p;
	}

	void ResourceBank::setImage(Image *p, const std::string &str)
	{
		imageResources[str] = p;
	}

	void ResourceBank::setModel(Model *p, const std::string &str)
	{
		modelResources[str] = p;
	}

	void ResourceBank::setPipeline(Pipeline *p, const std::string &str)
	{
		pipelineResources[str] = p;
	}

	void ResourceBank::setCmd(VkCommandBuffer p, const std::string &str)
	{
		cmdResources[str] = p;
	}

	Buffer *ResourceBank::getBuffer(const std::string &str)
	{
		auto it = bufferResources.find(str);
		if (it == bufferResources.end())
		{
			it = globalResource.bufferResources.find(str);
			if (it == globalResource.bufferResources.end())
				return nullptr;
		}
		return it->second;
	}

	Image *ResourceBank::getImage(const std::string &str)
	{
		auto it = imageResources.find(str);
		if (it == imageResources.end())
		{
			it = globalResource.imageResources.find(str);
			if (it == globalResource.imageResources.end())
				return nullptr;
		}
		return it->second;
	}

	Model *ResourceBank::getModel(const std::string &str)
	{
		auto it = modelResources.find(str);
		if (it == modelResources.end())
		{
			it = globalResource.modelResources.find(str);
			if (it == globalResource.modelResources.end())
				return nullptr;
		}
		return it->second;
	}

	Pipeline *ResourceBank::getPipeline(const std::string &str)
	{
		auto it = pipelineResources.find(str);
		if (it == pipelineResources.end())
		{
			it = globalResource.pipelineResources.find(str);
			if (it == globalResource.pipelineResources.end())
				return nullptr;
		}
		return it->second;
	}

	VkCommandBuffer ResourceBank::getCmd(const std::string &str)
	{
		auto it = cmdResources.find(str);
		if (it == cmdResources.end())
		{
			it = globalResource.cmdResources.find(str);
			if (it == globalResource.cmdResources.end())
				return nullptr;
		}
		return it->second;
	}
}