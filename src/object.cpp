#include "object.h"
#include "core.h"

namespace tke
{
	Object::Object(Model *_model, ObjectPhysicsType _physicsType)
		:model(_model), physicsType(_physicsType)
	{
		if (model->animated)
			animationComponent = new AnimationComponent(model);
	}

	Object::~Object()
	{
		delete animationComponent;
	}
}