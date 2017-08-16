#pragma once

#include "transformer.h"

namespace tke
{
	REFLECTABLE struct Water : Transformer
	{
		REFL_BANK;

		int blockCx = 64;
		float blockSize = 16.f;
		float height = 10.f;
		float tessellationFactor = 0.75f;
		float textureUvFactor = 8.f;
	};

}
