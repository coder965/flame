#include "resource.h"

namespace tke
{
	ResourceBank _resources;

	void ResourceBank::setBuffer(Buffer *p, const std::string &str)
	{
		bufferResources[str] = p;
	}

	void ResourceBank::setImage(Image *p, const std::string &str)
	{
		imageResources[str] = p;
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
			it = _resources.bufferResources.find(str);
			if (it == _resources.bufferResources.end())
				return nullptr;
		}
		return it->second;
	}

	Image *ResourceBank::getImage(const std::string &str)
	{
		auto it = imageResources.find(str);
		if (it == imageResources.end())
		{
			it = _resources.imageResources.find(str);
			if (it == _resources.imageResources.end())
				return nullptr;
		}
		return it->second;
	}

	Pipeline *ResourceBank::getPipeline(const std::string &str)
	{
		auto it = pipelineResources.find(str);
		if (it == pipelineResources.end())
		{
			it = _resources.pipelineResources.find(str);
			if (it == _resources.pipelineResources.end())
				return nullptr;
		}
		return it->second;
	}

	VkCommandBuffer ResourceBank::getCmd(const std::string &str)
	{
		auto it = cmdResources.find(str);
		if (it == cmdResources.end())
		{
			it = _resources.cmdResources.find(str);
			if (it == _resources.cmdResources.end())
				return nullptr;
		}
		return it->second;
	}

	ResourceBank *resources(ResourceBank *b)
	{
		if (b) return b;
		return &_resources;
	}
}