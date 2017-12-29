#pragma once

#include <string>
#include <memory>

#include "../refl.h"

namespace tke
{
	enum { MaxMaterialCount = 256 };
	enum { MaxMaterialImageCount = 256 };

	struct UniformBuffer;
	struct Image;
	struct DescriptorSet;

	struct Material
	{
		std::string name;

		unsigned char albedoR = 255, albedoG = 255, albedoB = 255;
		unsigned char alpha = 255;
		unsigned char spec = 0;
		unsigned char roughness = 255;

		std::shared_ptr<Image> albedoAlphaMap;
		std::shared_ptr<Image> normalHeightMap;
		std::shared_ptr<Image> specRoughnessMap;

		int sceneIndex = -1;
	};

	IMPL() std::weak_ptr<Material> materials[MaxMaterialCount];
	IMPL(nullptr) std::shared_ptr<Material> defaultMaterial;
	std::shared_ptr<Material> getMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB,
		unsigned char alpha, unsigned char spec, unsigned char roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap, std::shared_ptr<Image> specRoughnessMap);
	std::shared_ptr<Material> getMaterial(const std::string name);
	IMPL(nullptr) UniformBuffer *materialBuffer;

	IMPL() std::weak_ptr<Image> materialImages[MaxMaterialImageCount];
	std::shared_ptr<Image> getMaterialImage(const std::string &filename, bool sRGB = false);
	IMPL(nullptr) DescriptorSet *ds_material_images;

	void init_material();
}
