#pragma once

#include <memory>
#include <vector>

#include <flame/engine/graphics/graphics.h>

namespace flame
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
	std::shared_ptr<DescriptorSetLayout> get_descriptor_set_layout(const std::vector<DescriptorSetLayoutBinding> &_bindings);

	struct Pipeline;
	struct Buffer;
	struct Texture;

	VkDescriptorBufferInfo get_buffer_info(Buffer *b, int offset = 0, int range = 0);
	VkDescriptorImageInfo get_texture_info(Texture *t, VkSampler sampler, int base_level = 0, int level_count = 0, int base_layer = 0, int layer_count = 0, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D);

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
		VkWriteDescriptorSet get_write(int binding, int index, VkDescriptorBufferInfo *info);
		VkWriteDescriptorSet get_write(int binding, int index, VkDescriptorImageInfo *info);
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
