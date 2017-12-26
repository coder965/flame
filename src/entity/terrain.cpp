#include "terrain.h"
#include "../render/image.h"

namespace tke
{
	Terrain::Terrain(bool _use_physx, std::shared_ptr<Image> _blendMap)
		:Node(NodeTypeTerrain), use_physx(_use_physx), blendMap(_blendMap)
	{
		for (int i = 0; i < 4; i++)
			materials[i] = default_material;
	}

}
