#pragma once

#include <string>
#include <memory>

#include "../refl.h"
#include "../math/math.h"

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

		glm::vec4 albedo_alpha = glm::vec4(1.f);
		float spec = 0.f;
		float roughness = 1.f;

		std::shared_ptr<Image> albedoAlphaMap;
		std::shared_ptr<Image> normalHeightMap;
		std::shared_ptr<Image> specRoughnessMap;

		int index = -1;
	};

	IMPL() std::weak_ptr<Material> materials[MaxMaterialCount];
	IMPL() std::shared_ptr<Material> defaultMaterial;
	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, float spec, float roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap, 
		std::shared_ptr<Image> specRoughnessMap);
	std::shared_ptr<Material> getMaterial(const std::string name);
	IMPL(nullptr) UniformBuffer *materialBuffer;

	IMPL() std::weak_ptr<Image> materialImages[MaxMaterialImageCount];
	std::shared_ptr<Image> getMaterialImage(const std::string &filename, bool sRGB = false);
	IMPL(nullptr) DescriptorSet *ds_material;

	void init_material();
}
