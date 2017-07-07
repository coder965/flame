#pragma once

#include "transformer.h"
#include "render.h"

namespace tke
{
	struct Terrain : Transformer
	{
		int patchSize = 64;
		float ext = 10.f;
		float height = 10.f;
		float tessFactor = 0.75f;

		Image *heightMap = nullptr;
		Image *colorMap = nullptr;
		float spec = 0.04f;
		float roughness = 1.f;
	};
}
