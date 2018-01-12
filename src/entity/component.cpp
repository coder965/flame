#include "node.h"
#include "component.h"

namespace tke
{
	Component::Component(ComponentType _type) :
		type(_type)
	{
	}

	Component::~Component()
	{
		if (parent)
			parent->remove_component(this);
	}

	Node *Component::get_parent() const
	{
		return parent;
	}
}
