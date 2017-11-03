#include "terrain.h"
#include "../render/image.h"

namespace tke
{
	Terrain::Terrain(bool _use_physx, Image *_heightMap, Image *_normalMap, Image *_blendMap,
		Image *_colorMap0, Image *_colorMap1, Image *_colorMap2, Image *_colorMap3,
		Image *_normalMap0, Image *_normalMap1, Image *_normalMap2, Image *_normalMap3)
		:use_physx(_use_physx), blendMap(_blendMap), normalMap(_normalMap), heightMap(_heightMap)
	{
		colorMaps[0] = _colorMap0;
		colorMaps[1] = _colorMap1;
		colorMaps[2] = _colorMap2;
		colorMaps[3] = _colorMap3;
		normalMaps[0] = _normalMap0;
		normalMaps[1] = _normalMap1;
		normalMaps[2] = _normalMap2;
		normalMaps[3] = _normalMap3;
		if (heightMap)
			height_map_filename = heightMap->filename;
		if (normalMap)
			normal_map_filename = normalMap->filename;
		if (_colorMap0)
			color_map0_filename = _colorMap0->filename;
		if (_colorMap1)
			color_map1_filename = _colorMap1->filename;
		if (_colorMap2)
			color_map2_filename = _colorMap2->filename;
		if (_colorMap3)
			color_map3_filename = _colorMap2->filename;
		if (_normalMap0)
			normal_map0_filename = _normalMap0->filename;
		if (_normalMap1)
			normal_map1_filename = _normalMap1->filename;
		if (_normalMap2)
			normal_map2_filename = _normalMap2->filename;
		if (_normalMap3)
			normal_map3_filename = _normalMap3->filename;
	}

}
