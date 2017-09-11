#pragma once

namespace tke
{
	struct Image;
	struct Material
	{
		unsigned char albedoR = 255, albedoG = 255, albedoB = 255;
		unsigned char alpha = 255;
		unsigned char spec = 0;
		unsigned char roughness = 255;

		Image *albedoAlphaMap = nullptr;
		Image *normalHeightMap = nullptr;
		Image *specRoughnessMap = nullptr;

		int sceneIndex = -1;
	};

	struct Geometry
	{
		Material *material = nullptr;

		int indiceBase = 0; // offset of model base
		int indiceCount = 0;

		bool visible = true;
	};
}
