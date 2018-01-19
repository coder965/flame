#pragma once

#include <string>
#include <memory>

#include "../math/math.h"
#include "../spare_list.h"

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
		glm::vec2 spec_roughness = glm::vec2(0.f, 1.f);

		std::shared_ptr<Image> albedo_alpha_map;
		std::shared_ptr<Image> spec_roughness_map;
		std::shared_ptr<Image> normal_height_map;

		int index = -1;

		std::shared_ptr<Image> get_albedo_alpha_map() const;
		std::shared_ptr<Image> get_spec_roughness_map() const;
		std::shared_ptr<Image> get_normal_height_map() const;

		void set_albedo_alpha_map(std::shared_ptr<Image> i);
		void set_spec_roughness_map(std::shared_ptr<Image> i);
		void set_normal_height_map(std::shared_ptr<Image> i);
	};

	extern std::shared_ptr<Material> default_material;
	extern UniformBuffer *materialBuffer;
	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, glm::vec2 spec_roughness,
		std::shared_ptr<Image> albedoAlphaMap, std::shared_ptr<Image> specRoughnessMap, 
		std::shared_ptr<Image> normalHeightMap);
	std::shared_ptr<Material> getMaterial(const std::string name);

	std::shared_ptr<Image> getMaterialImage(const std::string &filename);

	extern DescriptorSet *ds_material;

	void init_material();
}
