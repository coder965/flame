#include <assert.h>

#include "descriptor.h"
#include "pipeline.h"
#include "buffer.h"
#include "image.h"

namespace tke
{
	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(vk_device.v, v, nullptr);
	}

	static std::vector<std::weak_ptr<DescriptorSetLayout>> _descriptorSetLayouts;

	std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(int bindingCount, VkDescriptorSetLayoutBinding *bindings)
	{
		for (auto it = _descriptorSetLayouts.begin(); it != _descriptorSetLayouts.end(); )
		{
			auto l = it->lock();

			if (l)
			{
				if (l->bindings.size() == bindingCount)
				{
					bool same = true;
					for (auto i = 0; i < bindingCount; i++)
					{
						if (l->bindings[i].binding != bindings[i].binding || l->bindings[i].descriptorCount != bindings[i].descriptorCount ||
							l->bindings[i].descriptorType != bindings[i].descriptorType || l->bindings[i].stageFlags != bindings[i].stageFlags)
						{
							same = false;
							break;
						}
					}
					if (same)
						return l;
				}

				it++;
			}
			else
				it = _descriptorSetLayouts.erase(it);
		}

		auto l = std::make_shared<DescriptorSetLayout>();
		for (int i = 0; i < bindingCount; i++)
			l->bindings.push_back(bindings[i]);

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = bindingCount;
		info.pBindings = l->bindings.data();

		auto res = vkCreateDescriptorSetLayout(vk_device.v, &info, nullptr, &l->v);
		assert(res == VK_SUCCESS);

		_descriptorSetLayouts.push_back(l);
		return l;
	}

	DescriptorSet::DescriptorSet(Pipeline *pipeline, int index)
		:layout(pipeline->descriptorSetLayouts[index].get())
	{
		VkDescriptorSetAllocateInfo descriptorSetInfo = {};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = descriptorPool->v;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = &layout->v;

		auto res = vkAllocateDescriptorSets(vk_device.v, &descriptorSetInfo, &v);
		assert(res == VK_SUCCESS);
	}

	DescriptorSet::~DescriptorSet()
	{
		auto res = vkFreeDescriptorSets(vk_device.v, descriptorPool->v, 1, &v);
		assert(res == VK_SUCCESS);
	}

	VkWriteDescriptorSet DescriptorSet::bufferWrite(int binding, int index, Buffer *buffer)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &buffer->info;
		return write;
	}

	VkWriteDescriptorSet DescriptorSet::imageWrite(int binding, int index, Image *image, VkSampler sampler, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pImageInfo = image->getInfo(image->getView(baseLevel, levelCount, baseLayer, layerCount), sampler);
		return write;
	}

	void updateDescriptorSets(int count, VkWriteDescriptorSet *writes)
	{
		if (count <= 0)
			return;

		vk_device.mtx.lock();
		vkUpdateDescriptorSets(vk_device.v, count, writes, 0, nullptr);
		vk_device.mtx.unlock();
	}

	DescriptorPool::DescriptorPool()
	{
		VkDescriptorPoolSize descriptorPoolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048 },
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
		descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
		descriptorPoolInfo.maxSets = 64;
		auto res = vkCreateDescriptorPool(vk_device.v, &descriptorPoolInfo, nullptr, &v);
		assert(res == VK_SUCCESS);
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(vk_device.v, v, nullptr);
	}

	DescriptorPool *descriptorPool = nullptr;
}
