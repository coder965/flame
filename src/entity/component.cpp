#include "node.h"
#include "component.h"

namespace tke
{
	Component::Component(ComponentType _type) :
		type(_type)
	{
	}

	Component::~Component() {}

	ComponentType Component::get_type() const
	{
		return type;
	}

	Node *Component::get_parent() const
	{
		return parent;
	}
}
