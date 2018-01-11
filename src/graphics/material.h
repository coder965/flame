#pragma once

#include <string>
#include <memory>

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

	extern std::weak_ptr<Material> materials[MaxMaterialCount];
	extern std::shared_ptr<Material> defaultMaterial;
	extern UniformBuffer *materialBuffer;
	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, float spec, float roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> normalHeightMap, 
		std::shared_ptr<Image> specRoughnessMap);
	std::shared_ptr<Material> getMaterial(const std::string name);

	extern std::weak_ptr<Image> materialImages[MaxMaterialImageCount];
	std::shared_ptr<Image> getMaterialImage(const std::string &filename);

	extern DescriptorSet *ds_material;

	void init_material();
}
