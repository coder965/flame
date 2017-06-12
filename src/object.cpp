#include "object.h"
#include "core.h"

namespace tke
{
	Object::Object(Model *_model)
		:model(_model)
	{
		if (model->animated)
			animationComponent = new AnimationComponent(model);
	}

	Object::~Object()
	{
		delete animationComponent;
	}
}