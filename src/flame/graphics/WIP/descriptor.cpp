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

	VkDescriptorBufferInfo get_buffer_info(Buffer *b, int offset, int range)
	{
		VkDescriptorBufferInfo i;
		i.buffer = b->v;
		i.offset = offset;
		i.range = range == 0 ? b->size : range;
		return i;
	}

	VkDescriptorImageInfo get_texture_info(Texture *t, VkSampler sampler, int base_level, int level_count, int base_layer, int layer_count, VkImageViewType view_type)
	{
		VkDescriptorImageInfo i;
		if (t->type == TextureTypeImage)
			i.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		else
		{
			if (get_format_type(t->format) == FormatTypeColor)
				i.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			else
				i.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		if (level_count == 0)
			level_count = t->levels.size();
		if (layer_count == 0)
			layer_count = t->layer_count;
		i.imageView = t->get_view(view_type, base_level, level_count, base_layer, layer_count);
		i.sampler = sampler;
		return i;
	}

	VkWriteDescriptorSet DescriptorSet::get_write(int binding, int index, VkDescriptorBufferInfo *info)
	{
		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = info;
		write.pImageInfo = nullptr;
		write.pTexelBufferView = nullptr;
		return write;
	}

	VkWriteDescriptorSet DescriptorSet::get_write(int binding, int index, VkDescriptorImageInfo *info)
	{
		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = v;
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pBufferInfo = nullptr;
		write.pImageInfo = info;
		write.pTexelBufferView = nullptr;
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
}
