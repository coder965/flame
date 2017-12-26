#include "../render/buffer.h"
#include "../render/image.h"
#include "material.h"

namespace tke
{
	struct MaterialShaderStruct
	{
		unsigned int albedoAlphaCompress;
		unsigned int specRoughnessCompress;

		unsigned int mapIndex;

		unsigned int dummy;
	};

	static void _update_material(Material *m, int index)
	{
		auto map = (unsigned char*)stagingBuffer->map(0, sizeof(MaterialShaderStruct));
		MaterialShaderStruct stru;
		stru.albedoAlphaCompress = m->albedoR + (m->albedoG << 8) + (m->albedoB << 16) + (m->alpha << 24);
		stru.specRoughnessCompress = m->spec + (m->roughness << 8);
		stru.mapIndex = (m->albedoAlphaMap ? m->albedoAlphaMap->index + 1 : 0) +
			((m->normalHeightMap ? m->normalHeightMap->index + 1 : 0) << 8) +
			((m->specRoughnessMap ? m->specRoughnessMap->index + 1 : 0) << 16);
		memcpy(map, &stru, sizeof(MaterialShaderStruct));
		stagingBuffer->unmap();

		VkBufferCopy range = {};
		range.srcOffset = 0;
		range.dstOffset = sizeof(MaterialShaderStruct) * index;
		range.size = sizeof(MaterialShaderStruct);

		stagingBuffer->copyTo(materialBuffer, 1, &range);
	}

	std::shared_ptr<Material> getMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB,
		unsigned char alpha, unsigned char spec, unsigned char roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap, std::shared_ptr<Image> specRoughnessMap)
	{
		for (int i = 0; i < MaxMaterialCount; i++)
		{
			auto m = materials[i].lock();
			if (m)
			{
				if (m->albedoAlphaMap != albedoAlphaMap ? false : (!albedoAlphaMap && m->albedoR == albedoR && m->albedoG == albedoG && m->albedoB == albedoB && m->alpha == alpha)
					&& m->specRoughnessMap != specRoughnessMap ? false : (!specRoughnessMap && m->spec == spec && m->roughness == roughness)
					&& m->normalHeightMap == normalHeightMap)
					return m;
			}
		}

		for (int i = 0; i < MaxMaterialCount; i++)
		{
			if (!materials[i].lock())
			{
				auto m = std::make_shared<Material>();
				m->albedoR = albedoR;
				m->albedoG = albedoG;
				m->albedoB = albedoB;
				m->alpha = alpha;
				m->spec = spec;
				m->roughness = roughness;
				m->albedoAlphaMap = albedoAlphaMap;
				m->normalHeightMap = normalHeightMap;
				m->specRoughnessMap = specRoughnessMap;
				m->sceneIndex = i;
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

	std::shared_ptr<Image> getMaterialImage(const std::string &_filename, bool sRGB)
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
				auto t = getImage(_filename, 0, sRGB);
				if (!t)
					return nullptr;

				t->index = i;
				modelTextures[i] = t;

				updateDescriptorSets(1, &ds_textures->imageWrite(0, i, t.get(), colorSampler));

				return t;
			}
		}
	}

	void init_material()
	{

	}
}
