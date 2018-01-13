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
		ComponentType type;
		Node *parent;
		friend class Node;
	public:
		Component(ComponentType _type);
		virtual ~Component() {}

		ComponentType get_type() const;
		Node *get_parent() const;
	protected:
		virtual void on_update() {};
	};
}
