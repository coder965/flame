#pragma once

#include <memory>
#include <vector>

#include "graphics.h"

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

		// must call in main thread
		~DescriptorSetLayout();
	};

	// must call in main thread
	std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(int bindingCount, VkDescriptorSetLayoutBinding *bindings);

	struct Pipeline;
	struct Buffer;
	struct Image;
	struct DescriptorSet
	{
		DescriptorSetLayout *layout;
		VkDescriptorSet v;

		// must call in main thread
		DescriptorSet(DescriptorSetLayout *_layout);
		// must call in main thread
		DescriptorSet(Pipeline *pipeline, int index = 0);
		// must call in main thread
		~DescriptorSet();
		VkWriteDescriptorSet bufferWrite(int binding, int index, Buffer *buffer);
		VkWriteDescriptorSet imageWrite(int binding, int index, Image *image, VkSampler sampler, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	void updateDescriptorSets(int count, VkWriteDescriptorSet *writes);

	struct DescriptorPool
	{
		VkDescriptorPool v;

		// must call in main thread
		DescriptorPool();
		// must call in main thread
		~DescriptorPool();
	};

	extern DescriptorPool *descriptorPool;
}
