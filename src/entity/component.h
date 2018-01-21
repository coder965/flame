#pragma once

#include "../object.h"

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
	struct XMLNode;

	class Component : public Object
	{
	private:
		ComponentType type;
		Node *parent;
		friend class Node;
	public:
		Component(ComponentType _type);
		virtual ~Component();

		ComponentType get_type() const;
		Node *get_parent() const;

		virtual void serialize(XMLNode *dst) {};
		virtual bool unserialize(XMLNode *src) {};
	protected:
		virtual void on_update() {};
	};
}
