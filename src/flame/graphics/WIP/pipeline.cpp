#include <flame/global.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/graphics/shader.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/resource.h>
#include <flame/engine/graphics/pipeline.h>

namespace flame
{
	bool operator==(const PushConstantRange &lhs, const PushConstantRange &rhs)
	{
		return lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.stage == rhs.stage;
	}

	PipelineInfo &PipelineInfo::add_uniform_buffer_link(const std::string &descriptor_name,
		const std::string &resource_name, int array_element,
		int offset, int range)
	{
		UniformBufferResourceLink l;
		l.binding = -1;
		l.descriptor_name = descriptor_name;
		l.resource_name = resource_name;
		l.array_element = array_element;
		l.offset = offset;
		l.range = range;
		uniform_buffer_links.push_back(l);
		return *this;
	}

	PipelineInfo &PipelineInfo::add_texture_link(const std::string &descriptor_name,
		const std::string &resource_name, int array_element,
		VkSampler sampler, int base_level, int level_count,
		int base_layer, int layer_count, VkImageViewType view_type)
	{
		TextureResourceLink l;
		l.binding = -1;
		l.descriptor_name = descriptor_name;
		l.resource_name = resource_name;
		l.array_element = array_element;
		l.sampler = sampler;
		l.base_level = base_level;
		l.level_count = level_count;
		l.base_array = base_layer;
		l.array_count = layer_count;
		l.view_type = view_type;
		texture_links.push_back(l);
		return *this;
	}

	PipelineLayout::PipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>> &_descriptor_set_layouts, const std::vector<PushConstantRange> &_push_constant_ranges) :
		descriptor_set_layouts(_descriptor_set_layouts),
		push_constant_ranges(_push_constant_ranges)
	{
		std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts(descriptor_set_layouts.size());
		for (auto i = 0; i < descriptor_set_layouts.size(); i++)
			vk_descriptor_set_layouts[i] = descriptor_set_layouts[i]->v;

		std::vector<VkPushConstantRange> vk_push_constant_ranges(push_constant_ranges.size());
		for (auto i = 0; i < push_constant_ranges.size(); i++)
		{
			vk_push_constant_ranges[i].offset = push_constant_ranges[i].offset;
			vk_push_constant_ranges[i].size = push_constant_ranges[i].size;
			vk_push_constant_ranges[i].stageFlags = push_constant_ranges[i].stage;
		}
	}

	static std::vector<std::weak_ptr<PipelineLayout>> pipelineLayouts;

	std::shared_ptr<PipelineLayout> get_pipeline_layout(const std::vector<std::shared_ptr<DescriptorSetLayout>> &_descriptor_set_layouts, const std::vector<PushConstantRange> &_push_constant_ranges)
	{
		bool found = false;
		for (auto it = pipelineLayouts.begin(); it != pipelineLayouts.end(); )
		{
			auto p = it->lock();

			if (p)
			{
				if (p->descriptor_set_layouts == _descriptor_set_layouts && p->push_constant_ranges == _push_constant_ranges)
					return p;
				it++;
			}
			else
				it = pipelineLayouts.erase(it);
		}

		auto p = std::make_shared <PipelineLayout>(_descriptor_set_layouts, _push_constant_ranges);
		pipelineLayouts.push_back(p);
		return p;
	}

	Pipeline::Pipeline(const PipelineInfo &_info, RenderPass *_render_pass, int _subpass_index, bool need_default_ds) :
		render_pass(_render_pass),
		subpass_index(_subpass_index),
		v(0),
		descriptor_set(nullptr)
	{
		info = _info;

		for (auto &_s : info.shaders)
		{
			auto s = get_shader(_s.first, _s.second, this);
			shaders.push_back(s);
		}

		create();

		if (need_default_ds)
			descriptor_set = std::make_unique<DescriptorSet>(this);

		if (descriptor_set)
			link_descriptors(descriptor_set.get(), &globalResource);
	}

	void Pipeline::create()
	{
		for (auto &s : shaders)
		{
			if (descriptor_sets.size() < s->descriptor_sets.size())
			{
				descriptor_sets.resize(s->descriptor_sets.size());
				descriptor_set_layouts.resize(s->descriptor_sets.size());
			}

			for (auto set = 0; set < s->descriptor_sets.size(); set++)
			{
				for (auto &d : s->descriptor_sets[set])
				{
					auto found = false;
					for (auto &b : descriptor_sets[set])
					{
						if (b.binding == d->binding)
						{
							b.stage |= s->stage;
							found = true;
							break;
						}
					}
					if (found)
						continue;

					descriptor_sets[set].push_back(d->get_layout_binding(s->stage));
				}
			}
			if (s->push_constant_size > 0)
			{
				auto found = false;
				for (auto &p : push_constant_ranges)
				{
					if (p.size == s->push_constant_size)
					{
						p.stage |= s->stage;
						found = true;
						break;
					}
				}
				if (!found)
				{
					PushConstantRange r;
					r.offset = 0;
					r.size = s->push_constant_size;
					r.stage = s->stage;
					push_constant_ranges.push_back(r);
				}
			}
		}

		for (auto set = 0; set < descriptor_sets.size(); set++)
			descriptor_set_layouts[set] = get_descriptor_set_layout(descriptor_sets[set]);

		pipeline_layout = get_pipeline_layout(descriptor_set_layouts, push_constant_ranges);
	}

	void Pipeline::link_descriptors(DescriptorSet *set, Resource *resource)
	{
		std::vector<VkWriteDescriptorSet> writes;

		auto prepare_link = [&](ResourceLink &link) {
			if (link.binding == -1)
			{
				bool found = false;
				for (auto &s : shaders)
				{
					if (found) 
						break;

					for (auto set = 0; set < s->descriptor_sets.size(); set++)
					{
						for (auto &d : s->descriptor_sets[set])
						{
							if (d->name == link.descriptor_name)
							{
								link.binding = d->binding;
								found = true;
								break;
							}
						}
					}
				}
				if (!found)
					int cut = 1;
				//assert(found);
			}
		};

		std::vector<VkDescriptorBufferInfo> buffer_infos(info.uniform_buffer_links.size());
		for (auto i = 0; i < info.uniform_buffer_links.size(); i++)
		{
			auto &link = info.uniform_buffer_links[i];

			prepare_link(link);

			auto buffer = resource->getBuffer(link.resource_name);
			if (buffer)
			{
				buffer_infos[i] = get_buffer_info(buffer, link.offset, link.range);
				writes.push_back(set->get_write(link.binding, link.array_element, &buffer_infos[i]));
			}
			else
				printf("unable to link resource %s (binding:%d, type:uniform buffer)\n", link.resource_name.c_str(), link.binding);
		}

		std::vector<VkDescriptorImageInfo> image_infos(info.texture_links.size());
		for (auto i = 0; i < info.texture_links.size(); i++)
		{
			auto &link = info.texture_links[i];

			prepare_link(link);

			auto image = resource->getImage(link.resource_name);
			if (image)
			{
				image_infos[i] = get_texture_info(image, link.sampler, link.base_level, link.level_count, link.base_array, link.array_count, link.view_type);
				writes.push_back(set->get_write(link.binding, link.array_element, &image_infos[i]));
			}
			else
				printf("unable to link resource %s (binding:%d, type:combined image sampler)\n", link.resource_name.c_str(), link.binding);
		}

		updateDescriptorSets(writes.size(), writes.data());
	}

	int Pipeline::descriptor_position(const std::string &name)
	{
		for (auto &s : shaders)
		{
			for (auto set = 0; set < s->descriptor_sets.size(); set++)
			{
				for (auto &d : s->descriptor_sets[set])
				{
					if (d->name == name)
						return d->binding;
				}
			}
		}
		return -1;
	}
}
