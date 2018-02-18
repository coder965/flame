#pragma once

#include <memory>
#include <vector>

#include <flame/graphics/graphics.h>

namespace tke
{
	struct DescriptorSetLayoutBinding
	{
		VkDescriptorType type;
		int binding;
		int count;
		VkShaderStageFlags stage;
	};

	bool operator==(const DescriptorSetLayoutBinding &lhs, const DescriptorSetLayoutBinding &rhs);

	struct Descriptor
	{
		VkDescriptorType type;
		int binding;
		int count;
		std::string name;

		Descriptor();
		DescriptorSetLayoutBinding get_layout_binding(VkShaderStageFlags stage) const;
	};

	struct DescriptorSetLayout
	{
		std::vector<DescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;

		// must call in main thread
		DescriptorSetLayout(const std::vector<DescriptorSetLayoutBinding> &_bindings);
		// must call in main thread
		~DescriptorSetLayout();
	};

	bool operator==(const DescriptorSetLayout &lhs, const DescriptorSetLayout &rhs);

	// must call in main thread
	std::shared_ptr<DescriptorSetLayout> get_or_create_descriptor_set_layout(const std::vector<DescriptorSetLayoutBinding> &_bindings);

	struct Pipeline;
	struct Buffer;
	struct Texture;
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
		VkWriteDescriptorSet imageWrite(int binding, int index, Texture *image, VkSampler sampler, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	void updateDescriptorSets(VkWriteDescriptorSet *writes);
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
