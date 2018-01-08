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
		stru.spec_roughness.spec = glm::clamp(m->spec, 0.f, 1.f) * 255.f;
		stru.spec_roughness.roughness = glm::clamp(m->roughness, 0.f, 1.f) * 255.f;
		stru.map_index.albedo_alpha = m->albedoAlphaMap ? m->albedoAlphaMap->material_index + 1 : 0;
		stru.map_index.normal_height = m->normalHeightMap ? m->normalHeightMap->material_index + 1 : 0;
		stru.map_index.spec_roughness = m->specRoughnessMap ? m->specRoughnessMap->material_index + 1 : 0;
		memcpy(map, &stru, sizeof(MaterialShaderStruct));
		defalut_staging_buffer->unmap();

		VkBufferCopy range = {};
		range.srcOffset = 0;
		range.dstOffset = sizeof(MaterialShaderStruct) * index;
		range.size = sizeof(MaterialShaderStruct);

		defalut_staging_buffer->copyTo(materialBuffer, 1, &range);
	}

	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, float spec, float roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap,
		std::shared_ptr<Image> specRoughnessMap)
	{
		for (int i = 0; i < MaxMaterialCount; i++)
		{
			auto m = materials[i].lock();
			if (m)
			{
				if (m->albedoAlphaMap != albedoAlphaMap ? false : (!albedoAlphaMap && fEqual(m->albedo_alpha, albedo_alpha))
					&& m->specRoughnessMap != specRoughnessMap ? false : (!specRoughnessMap && fEqual(m->spec, spec) && fEqual(m->roughness, roughness))
					&& m->normalHeightMap == normalHeightMap)
					return m;
			}
		}

		for (int i = 0; i < MaxMaterialCount; i++)
		{
			if (!materials[i].lock())
			{
				auto m = std::make_shared<Material>();
				m->albedo_alpha = albedo_alpha;
				m->spec = spec;
				m->roughness = roughness;
				m->albedoAlphaMap = albedoAlphaMap;
				m->normalHeightMap = normalHeightMap;
				m->specRoughnessMap = specRoughnessMap;
				m->index = i;
				materials[i] = m;

				_update_material(m.get(), i);

				return m;
			}
		}
	}

	std::shared_ptr<Material> getMaterial(const std::string name)
	{
		for (int i = 0; i < MaxMaterialCount; i++)
		{
			auto m = materials[i].lock();
			if (m && m->name == name)
				return m;
		}
		return std::shared_ptr<Material>();
	}

	std::shared_ptr<Image> getMaterialImage(const std::string &_filename)
	{
		for (int i = 0; i < MaxMaterialImageCount; i++)
		{
			auto t = materialImages[i].lock();
			if (t)
			{
				if (t->filename == _filename)
					return t;
			}
		}

		for (int i = 0; i < MaxMaterialImageCount; i++)
		{
			if (!materialImages[i].lock())
			{
				auto t = get_image(_filename);
				if (!t)
					return nullptr;

				t->material_index = i;
				materialImages[i] = t;

				updateDescriptorSets(1, &ds_material->imageWrite(MaterialImagesDescriptorBinding, i, t.get(), colorSampler));

				return t;
			}
		}
	}

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

		defaultMaterial = std::make_shared<Material>();
		defaultMaterial->name = "[default_material]";
		defaultMaterial->index = 0;
		materials[0] = defaultMaterial;
		_update_material(defaultMaterial.get(), 0);

		updateDescriptorSets(1, &ds_material->bufferWrite(MaterialBufferDescriptorBinding, 0, materialBuffer));
	}
}
