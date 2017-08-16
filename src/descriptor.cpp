#include "descriptor.h"

namespace tke
{
	DescriptorSet::DescriptorSet(DescriptorPool *_pool, Pipeline *pipeline, int index)
		:pool(_pool), layout(pipeline->descriptorSetLayouts[index])
	{
		VkDescriptorSetAllocateInfo descriptorSetInfo = {};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = pool->v;
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
		auto res = vkFreeDescriptorSets(device.v, pool->v, 1, &v);
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

	void DescriptorSet::setImage(int binding, int index, Image *image, VkSampler sampler, VkImageAspectFlags aspect, int baseLevel, int levelCount, int baseLayer, int layerCount)
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
		info.imageView = image->getView(aspect, baseLevel, levelCount, baseLayer, layerCount);
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

	DescriptorPool *descriptorPool = nullptr;

}
