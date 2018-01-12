#pragma once

#include "../_object.h"

namespace tke
{
	enum ComponentType
	{
		ComponentTypeController,
		ComponentTypeCamera,
		ComponentTypeLight,
		ComponentTypeModelInstance,
		ComponentTypeTerrain,
		ComponentTypeWater
	};

	class Node;

	class Component : public _Object
	{
	private:
		Node *parent;
		ComponentType type;
		friend class Node;
	public:
		Component(ComponentType _type);
		virtual ~Component();
		Node *get_parent() const;
	protected:
		virtual void on_update() {};
	};
}
