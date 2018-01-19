#include "../global.h"
#include "../resource/resource.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/descriptor.h"
#include "../graphics/sampler.h"
#include "material.h"

namespace tke
{
	struct MaterialShaderStruct
	{
		union
		{
			struct
			{
				unsigned char albedo_r;
				unsigned char albedo_g;
				unsigned char albedo_b;
				unsigned char alpha;
			};
			unsigned int packed;
		}albedo_alpha;

		union
		{
			struct
			{
				unsigned char spec;
				unsigned char roughness;
				unsigned char dummy0;
				unsigned char dummy1;
			};
			unsigned int packed;
		}spec_roughness;

		union
		{
			struct
			{
				unsigned char albedo_alpha;
				unsigned char normal_height;
				unsigned char spec_roughness;
				unsigned char dummy;
			};
			unsigned int packed;
		}map_index;

		unsigned int dummy;
	};

	static void _update_material(Material *m, int index)
	{
		auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(MaterialShaderStruct));
		MaterialShaderStruct stru;
		auto albedo_alpha = glm::clamp(m->albedo_alpha, 0.f, 1.f);
		stru.albedo_alpha.albedo_r = albedo_alpha.r * 255.f;
		stru.albedo_alpha.albedo_g = albedo_alpha.g * 255.f;
		stru.albedo_alpha.albedo_b = albedo_alpha.b * 255.f;
		stru.albedo_alpha.alpha = albedo_alpha.a * 255.f;
		stru.spec_roughness.spec = glm::clamp(m->spec_roughness.x, 0.f, 1.f) * 255.f;
		stru.spec_roughness.roughness = glm::clamp(m->spec_roughness.y, 0.f, 1.f) * 255.f;
		stru.map_index.albedo_alpha = m->albedo_alpha_map ? m->albedo_alpha_map->material_index + 1 : 0;
		stru.map_index.normal_height = m->normal_height_map ? m->normal_height_map->material_index + 1 : 0;
		stru.map_index.spec_roughness = m->spec_roughness_map ? m->spec_roughness_map->material_index + 1 : 0;
		memcpy(map, &stru, sizeof(MaterialShaderStruct));
		defalut_staging_buffer->unmap();

		VkBufferCopy range = {};
		range.srcOffset = 0;
		range.dstOffset = sizeof(MaterialShaderStruct) * index;
		range.size = sizeof(MaterialShaderStruct);

		defalut_staging_buffer->copyTo(materialBuffer, 1, &range);
	}

	std::shared_ptr<Image> Material::get_albedo_alpha_map() const
	{
		return albedo_alpha_map;
	}

	std::shared_ptr<Image> Material::get_spec_roughness_map() const
	{
		return spec_roughness_map;
	}

	std::shared_ptr<Image> Material::get_normal_height_map() const
	{
		return normal_height_map;
	}

	void Material::set_albedo_alpha_map(std::shared_ptr<Image> i)
	{
		if (albedo_alpha_map == i)
			return;

		albedo_alpha_map = i;
	}

	void Material::set_spec_roughness_map(std::shared_ptr<Image> i)
	{
		if (spec_roughness_map == i)
			return;

		spec_roughness_map = i;
	}
	void Material::set_normal_height_map(std::shared_ptr<Image> i)
	{
		if (normal_height_map == i)
			return;

		normal_height_map = i;
	}


	static SpareList _material_list(MaxMaterialCount);
	static std::weak_ptr<Material> _materials[MaxMaterialCount];
	std::shared_ptr<Material> default_material;
	UniformBuffer *materialBuffer = nullptr;

	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, glm::vec2 spec_roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap,
		std::shared_ptr<Image> specRoughnessMap)
	{
		std::shared_ptr<Material> m;
		_material_list.iterate([&](int index, void *p, bool &remove) {
			auto _m = _materials[index].lock();
			if (_m)
			{
				if (_m->albedo_alpha_map != albedoAlphaMap ? false : (!albedoAlphaMap && is_same(_m->albedo_alpha, albedo_alpha))
					&& _m->spec_roughness_map != specRoughnessMap ? false : (!specRoughnessMap && is_same(_m->spec_roughness, spec_roughness))
					&& _m->normal_height_map == normalHeightMap)
					m = _m;
			}
			else
			{
				_materials[index].reset();
				remove = true;
			}
		});

		if (!m)
		{
			m = std::make_shared<Material>();
			m->albedo_alpha = albedo_alpha;
			m->spec_roughness = spec_roughness;
			m->albedo_alpha_map = albedoAlphaMap;
			m->normal_height_map = normalHeightMap;
			m->spec_roughness_map = specRoughnessMap;
			auto index = _material_list.add(m.get());
			m->index = index;
			_materials[index] = m;
		}

		return m;
	}

	std::shared_ptr<Material> getMaterial(const std::string name)
	{
		std::shared_ptr<Material> m;
		_material_list.iterate([&](int index, void *p, bool &remove) {
			auto _m = _materials[index].lock();
			if (_m)
			{
				if (_m->name == name)
					m = _m;
			}
			else
			{
				_materials[index].reset();
				remove = true;
			}
		});

		return m ? m : default_material;
	}

	static SpareList _material_image_list(MaxMaterialImageCount);
	static std::weak_ptr<Image> _material_images[MaxMaterialImageCount];

	std::shared_ptr<Image> getMaterialImage(const std::string &_filename)
	{
		std::shared_ptr<Image> i;
		_material_image_list.iterate([&](int index, void *p, bool &remove) {
			auto _i = _material_images[index].lock();
			if (_i)
			{
				if (_i->filename == _filename)
					i = _i;
			}
			else
			{
				_material_images[index].reset();
				remove = true;
			}
		});

		if (!i)
		{
			i = get_image(_filename);
			if (!i)
				return nullptr;

			auto index = _material_image_list.add(i.get());
			i->material_index = index;
			_material_images[index] = i;

			updateDescriptorSets(1, &ds_material->imageWrite(MaterialImagesDescriptorBinding, index, i.get(), colorSampler));
		}

		return i;
	}

	DescriptorSet *ds_material = nullptr;

	static std::shared_ptr<DescriptorSetLayout> _material_layout;

	void init_material()
	{
		VkDescriptorSetLayoutBinding bindings[] = {
			{
				MaterialBufferDescriptorBinding,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				1,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				nullptr
			},
				{
					MaterialImagesDescriptorBinding,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					MaxMaterialCount,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
				}
		};
		_material_layout = getDescriptorSetLayout(TK_ARRAYSIZE(bindings), bindings);

		ds_material = new DescriptorSet(_material_layout.get());

		materialBuffer = new UniformBuffer(sizeof(MaterialShaderStruct) * MaxMaterialCount);

		default_material = std::make_shared<Material>();
		default_material->name = "[default_material]";
		default_material->index = 0;
		{
			auto index = _material_list.add(default_material.get());
			_materials[index] = default_material;
		}
		_update_material(default_material.get(), 0);

		updateDescriptorSets(1, &ds_material->bufferWrite(MaterialBufferDescriptorBinding, 0, materialBuffer));
	}
}
