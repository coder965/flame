#ifndef __TKE_TERRAIN__
#define __TKE_TERRAIN__

#include "transformer.h"

namespace tke
{
	struct Image;
	struct Terrain : Transformer
	{
		int refrenceCount = 0;
		bool dying = false;

		int patchSize = 64;
		float ext = 10.f;
		float height = 10.f;
		float tessFactor = 0.75f;

		Image *heightMap = nullptr;
		Image *colorMap = nullptr;
		float spec = 0.04f;
		float roughness = 1.f;

		void getRefrence();
		void release();
	};
}

#endif