#include <assert.h>

#include "descriptor.h"
#include "pipeline.h"
#include "buffer.h"
#include "image.h"

namespace tke
{
	DescriptorSetLayout::~DescriptorSetLayout()
	{
		device.mtx.lock();
		vkDestroyDescriptorSetLayout(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<DescriptorSetLayout*> _descriptorSetLayouts;

	DescriptorSetLayout *getDescriptorSetLayout(int bindingCount, VkDescriptorSetLayoutBinding *bindings)
	{
		for (auto l : _descriptorSetLayouts)
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
				{
					l->refCount++;
					return l;
				}
			}
		}

		auto l = new DescriptorSetLayout;
		for (int i = 0; i < bindingCount; i++)
			l->bindings.push_back(bindings[i]);

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = bindingCount;
		info.pBindings = l->bindings.data();

		device.mtx.lock();
		auto res = vkCreateDescriptorSetLayout(device.v, &info, nullptr, &l->v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		_descriptorSetLayouts.push_back(l);
		return l;
	}

	void releaseDescriptorSetLayout(DescriptorSetLayout *l)
	{
		l->refCount--;
		if (l->refCount == 0)
		{
			for (auto it = _descriptorSetLayouts.begin(); it != _descriptorSetLayouts.end(); it++)
			{
				if (*it == l)
				{
					_descriptorSetLayouts.erase(it);
					delete l;
					break;
				}
			}
		}
	}

	DescriptorSet::DescriptorSet(Pipeline *pipeline, int index)
		:layout(pipeline->descriptorSetLayouts[index])
	{
		VkDescriptorSetAllocateInfo descriptorSetInfo = {};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = descriptorPool->v;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = &layout->v;

		device.mtx.lock();
		auto res = vkAllocateDescriptorSets(device.v, &descriptorSetInfo, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	DescriptorSet::~DescriptorSet()
	{
		device.mtx.lock();
		auto res = vkFreeDescriptorSets(device.v, descriptorPool->v, 1, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	void DescriptorSet::setBuffer(int binding, int index, Buffer *buffer)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		VkDescriptorBufferInfo info;
		info.offset = 0;
		info.buffer = buffer->v;
		info.range = buffer->size;
		write.pBufferInfo = &info;

		device.mtx.lock();
		vkUpdateDescriptorSets(device.v, 1, &write, 0, nullptr);
		device.mtx.unlock();
	}

	void DescriptorSet::setImage(int binding, int index, Image *image, VkSampler sampler, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		VkDescriptorImageInfo info;
		info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		info.imageView = image->getView(baseLevel, levelCount, baseLayer, layerCount);
		info.sampler = sampler;
		write.pImageInfo = &info;

		device.mtx.lock();
		vkUpdateDescriptorSets(device.v, 1, &write, 0, nullptr);
		device.mtx.unlock();
	}

	DescriptorPool::DescriptorPool()
	{
		VkDescriptorPoolSize descriptorPoolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000 },
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
		descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
		descriptorPoolInfo.maxSets = 256;
		device.mtx.lock();
		auto res = vkCreateDescriptorPool(device.v, &descriptorPoolInfo, nullptr, &v);
		device.mtx.unlock();
		assert(res == VK_SUCCESS);
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(device.v, v, nullptr);
	}

	thread_local DescriptorPool *descriptorPool = nullptr;
}
