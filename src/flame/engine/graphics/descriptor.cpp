#include <assert.h>

#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/pipeline.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>

namespace flame
{
	bool operator==(const DescriptorSetLayoutBinding &lhs, const DescriptorSetLayoutBinding &rhs)
	{
		return lhs.type == rhs.type && lhs.binding == rhs.binding && lhs.count == rhs.count && lhs.stage == rhs.stage;
	}

	Descriptor::Descriptor() :
		type(VK_DESCRIPTOR_TYPE_MAX_ENUM),
		binding(0),
		count(0)
	{
	}

	DescriptorSetLayoutBinding Descriptor::get_layout_binding(VkShaderStageFlags stage) const
	{
		DescriptorSetLayoutBinding b;
		b.type = type;
		b.binding = binding;
		b.count = count;
		b.stage = stage;
		return b;
	}

	DescriptorSetLayout::DescriptorSetLayout(const std::vector<DescriptorSetLayoutBinding> &_bindings) :
		bindings(_bindings)
	{
		std::vector<VkDescriptorSetLayoutBinding> vk_bindings(bindings.size());
		for (auto i = 0; i < bindings.size(); i++)
		{
			vk_bindings[i].pImmutableSamplers = nullptr;
			vk_bindings[i].descriptorType = bindings[i].type;
			vk_bindings[i].binding = bindings[i].binding;
			vk_bindings[i].descriptorCount = bindings[i].count;
			vk_bindings[i].stageFlags = bindings[i].stage;
		}

		VkDescriptorSetLayoutCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;
		info.bindingCount = bindings.size();
		info.pBindings = vk_bindings.data();

		vk_chk_res(vkCreateDescriptorSetLayout(vk_device, &info, nullptr, &v));
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(vk_device, v, nullptr);
	}

	bool operator==(const DescriptorSetLayout &lhs, const DescriptorSetLayout &rhs)
	{
		return lhs.bindings == rhs.bindings && lhs.v == rhs.v;
	}

	static std::vector<std::weak_ptr<DescriptorSetLayout>> _descriptor_set_layouts;

	std::shared_ptr<DescriptorSetLayout> get_descriptor_set_layout(const std::vector<DescriptorSetLayoutBinding> &_bindings)
	{
		for (auto it = _descriptor_set_layouts.begin(); it != _descriptor_set_layouts.end(); )
		{
			auto l = it->lock();

			if (l)
			{
				if (l->bindings == _bindings)
					return l;

				it++;
			}
			else
				it = _descriptor_set_layouts.erase(it);
		}

		auto l = std::make_shared<DescriptorSetLayout>(_bindings);
		_descriptor_set_layouts.push_back(l);
		return l;
	}

	DescriptorSet::DescriptorSet(DescriptorSetLayout *_layout)
		:layout(_layout)
	{
		VkDescriptorSetAllocateInfo descriptorSetInfo = {};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = descriptorPool->v;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = &layout->v;

		vk_chk_res(vkAllocateDescriptorSets(vk_device, &descriptorSetInfo, &v));
	}

	DescriptorSet::DescriptorSet(Pipeline *pipeline, int index) :
		DescriptorSet(pipeline->pipeline_layout->descriptor_set_layouts[index].get())
	{
	}

	DescriptorSet::~DescriptorSet()
	{
		vk_chk_res(vkFreeDescriptorSets(vk_device, descriptorPool->v, 1, &v));
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

	VkWriteDescriptorSet DescriptorSet::imageWrite(int binding, int index, Texture *image, VkSampler sampler, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pImageInfo = image->get_info(image->get_view(VK_IMAGE_VIEW_TYPE_2D, baseLevel, levelCount, baseLayer, layerCount), sampler);
		return write;
	}

	void updateDescriptorSets(VkWriteDescriptorSet *writes)
	{
		updateDescriptorSets(1, writes);
	}

	void updateDescriptorSets(int count, VkWriteDescriptorSet *writes)
	{
		if (count <= 0)
			return;

		vkUpdateDescriptorSets(vk_device, count, writes, 0, nullptr);
	}

	DescriptorPool::DescriptorPool()
	{
		VkDescriptorPoolSize descriptorPoolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048 },
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
		descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
		descriptorPoolInfo.maxSets = 64;
		vk_chk_res(vkCreateDescriptorPool(vk_device, &descriptorPoolInfo, nullptr, &v));
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(vk_device, v, nullptr);
	}

	DescriptorPool *descriptorPool = nullptr;
}
