#pragma once

#include "vulkan.h"

namespace tke
{
	struct DescriptorSet
	{
		DescriptorPool *pool;
		DescriptorSetLayout *layout;
		VkDescriptorSet v;

		DescriptorSet(DescriptorPool *_pool, Pipeline *pipeline, int index = 0);
		~DescriptorSet();
		void setBuffer(int binding, int index, Buffer *buffer);
		void setImage(int binding, int index, Image *image, VkSampler sampler, VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	struct DescriptorPool
	{
		VkDescriptorPool v;

		DescriptorPool();
		~DescriptorPool();
	};
	extern DescriptorPool *descriptorPool;
}
