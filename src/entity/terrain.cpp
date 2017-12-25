#include "terrain.h"
#include "../render/image.h"

namespace tke
{
	Terrain::Terrain(bool _use_physx, std::shared_ptr<Image> _normalHeightMap, 
		std::shared_ptr<Image> _blendMap, std::shared_ptr<Image> _colorMap0, 
		std::shared_ptr<Image> _colorMap1, std::shared_ptr<Image> _colorMap2, 
		std::shared_ptr<Image> _colorMap3, std::shared_ptr<Image> _normalMap0, 
		std::shared_ptr<Image> _normalMap1, std::shared_ptr<Image> _normalMap2, 
		std::shared_ptr<Image> _normalMap3)
		:Node(NodeTypeTerrain), use_physx(_use_physx), blendMap(_blendMap), normalHeightMap(_normalHeightMap)
	{
		colorMaps[0] = _colorMap0;
		colorMaps[1] = _colorMap1;
		colorMaps[2] = _colorMap2;
		colorMaps[3] = _colorMap3;
		normalMaps[0] = _normalMap0;
		normalMaps[1] = _normalMap1;
		normalMaps[2] = _normalMap2;
		normalMaps[3] = _normalMap3;
	}

}
