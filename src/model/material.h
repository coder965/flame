#pragma once

#include <string>
#include <memory>

namespace tke
{
	struct Image;
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

	struct Geometry
	{
		std::shared_ptr<Material> material;

		int indiceBase = 0; // offset of model base
		int indiceCount = 0;

		bool visible = true;
	};
}
