#pragma once

namespace tke
{
	enum ComponentType
	{
		ComponentTypeCamera,
		ComponentTypeLight,
		ComponentTypeMesh,
		ComponentTypeTerrain,
		ComponentTypeWater
	};

	struct Component
	{
		ComponentType type;

		Component(ComponentType _type);
	};
}
