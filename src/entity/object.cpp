#include "object.h"

namespace tke
{
	ObjectRigidBodyData::~ObjectRigidBodyData()
	{
		if (actor)
			actor->release();
	}

	Object::Object()
		:Node(NodeTypeObject)
	{
	}

	Object::Object(std::shared_ptr<Model> _model, unsigned int _physicsType)
		: Node(NodeTypeObject), model(_model), physics_type(_physicsType)
	{
		model_filename = model->filename;
		if (model->vertex_skeleton)
			animationComponent = std::make_unique<AnimationComponent>(model.get());
	}

	Object::~Object()
	{
		if (pxController)
			pxController->release();
	}

	void Object::setState(Controller::State _s, bool enable)
	{
		if (Controller::setState(_s, enable))
		{
			if (animationComponent)
			{
				if (state == Controller::State::stand)
					animationComponent->setAnimation(model->stateAnimations[ModelStateAnimationStand].get());
				else if (state == Controller::State::forward)
					animationComponent->setAnimation(model->stateAnimations[ModelStateAnimationForward].get());
			}
		}
	}
}
