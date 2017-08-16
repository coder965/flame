#pragma once

#include "utils.h"
#include "vulkan.h"

namespace tke
{
	REFLECTABLE enum class DescriptorType : int
	{
		null,
		REFLe uniform_buffer = 1 << 0,
		REFLe image_n_sampler = 1 << 1
	};

	REFLECTABLE struct Descriptor
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int count = 0;
		REFLv std::string name;
	};

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;
		int refCount = 1;

		~DescriptorSetLayout();
	};

	DescriptorSetLayout *getDescriptorSetLayout(int bindingCount, VkDescriptorSetLayoutBinding *bindings);

	void releaseDescriptorSetLayout(DescriptorSetLayout *l);

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
		void setImage(int binding, int index, Image *image, VkSampler sampler, VkImageAspectFlags aspect = 0, int baseLevel = 0, int levelCount = 1, int baseLayer = 0, int layerCount = 1);
	};

	struct DescriptorPool
	{
		VkDescriptorPool v;

		DescriptorPool();
		~DescriptorPool();
	};

	thread_local extern DescriptorPool *descriptorPool;
}
