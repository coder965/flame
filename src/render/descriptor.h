#pragma once

#include <memory>

#include "../utils.h"
#include "vulkan.h"

namespace tke
{
	struct Descriptor
	{
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		int binding = 0;
		int count = 0;
		std::string name;
	};

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;

		~DescriptorSetLayout();
	};

	std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(int bindingCount, VkDescriptorSetLayoutBinding *bindings);

	struct Pipeline;
	struct Buffer;
	struct Image;
	struct DescriptorSet
	{
		DescriptorSetLayout *layout;
		VkDescriptorSet v;

		DescriptorSet(Pipeline *pipeline, int index = 0);
		~DescriptorSet();
		void setBuffer(int binding, int index, Buffer *buffer);
		void setImage(int binding, int index, Image *image, VkSampler sampler, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	struct DescriptorPool
	{
		VkDescriptorPool v;

		DescriptorPool();
		~DescriptorPool();
	};

	thread_local extern DescriptorPool *descriptorPool;
}
