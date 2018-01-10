#pragma once

namespace tke
{
	enum ComponentType
	{
		ComponentTypeController,
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
		virtual void update() = 0;
	};
}
