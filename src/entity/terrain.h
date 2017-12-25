#pragma once

#include "transformer.h"
#include "node.h"

namespace physx { struct PxRigidActor; }

namespace tke
{
	struct Image;
	REFLECTABLE struct Terrain : Transformer, Node
	{
		REFL_BANK;

		REFLv bool use_physx = false;

		REFLv int block_cx = 64;
		REFLv float block_size = 16.f;
		REFLv float height = 100.f;
		REFLv float tessellation_factor = 0.75f;
		REFLv float texture_uv_factor = 8.f;

		std::shared_ptr<Image> normalHeightMap;
		std::shared_ptr<Image> blendMap;
		std::shared_ptr<Image> colorMaps[4];
		std::shared_ptr<Image> normalMaps[4];
		float spec = 0.04f;
		float roughness = 1.f;

		physx::PxRigidActor *actor = nullptr;

		int sceneIndex = -1;

		Terrain(bool _use_physx = false, std::shared_ptr<Image> _normalHeightMap = nullptr, 
			std::shared_ptr<Image> _blendMap = nullptr, std::shared_ptr<Image> _colorMap0 = nullptr, 
			std::shared_ptr<Image> _colorMap1 = nullptr, std::shared_ptr<Image> _colorMap2 = nullptr, 
			std::shared_ptr<Image> _colorMap3 = nullptr, std::shared_ptr<Image> _normalMap0 = nullptr, 
			std::shared_ptr<Image> _normalMap1 = nullptr, std::shared_ptr<Image> _normalMap2 = nullptr, 
			std::shared_ptr<Image> _normalMap3 = nullptr);
	};

}
