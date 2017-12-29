#include "../graphics/image.h"
#include "../graphics/material.h"
#include "terrain.h"

namespace tke
{
	Terrain::Terrain(bool _use_physx, std::shared_ptr<Image> _blendMap)
		:Node(NodeTypeTerrain), use_physx(_use_physx), blendMap(_blendMap)
	{
		if (!blendMap)
			 blendMap = default_blend_image;

		for (int i = 0; i < 4; i++)
			materials[i] = defaultMaterial;
	}
}
