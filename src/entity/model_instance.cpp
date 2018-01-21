//#include "../graphics/buffer.h"
//#include "../model/animation.h"
//#include "../physics/physics.h"
#include "component.h"
#include "../model/model.h"
#include "model_instance.h"

namespace tke
{
	void ModelInstanceComponent::serialize(XMLNode *dst)
	{

	}

	ModelInstanceComponent::ModelInstanceComponent(std::shared_ptr<Model> _model) :
		Component(ComponentTypeModelInstance),
		model(_model),
		instance_index(-1)
	{
	}

	Model *ModelInstanceComponent::get_model() const
	{
		return model.get();
	}

	int ModelInstanceComponent::get_instance_index() const
	{
		return instance_index;
	}

	void ModelInstanceComponent::set_instance_index(int v)
	{
		instance_index = v;
	}

	//ObjectRigidBodyData::~ObjectRigidBodyData()
	//{
	//	if (actor)
	//		actor->release();
	//}

	//Object::Object()
	//	:Node(NodeTypeObject)
	//{
	//}

	//Object::Object(std::shared_ptr<Model> _model, unsigned int _physicsType)
	//	: Node(NodeTypeObject), model(_model), physics_type(_physicsType)
	//{
	//	model_filename = model->filename;
	//	if (model->vertex_skeleton)
	//		animationComponent = std::make_unique<AnimationRunner>(model.get());
	//}

	//Object::~Object()
	//{
	//	if (pxController)
	//		pxController->release();
	//}

	//void Object::setState(Controller::State _s, bool enable)
	//{
	//	if (Controller::setState(_s, enable))
	//	{
	//		if (animationComponent)
	//		{
	//			if (state == Controller::State::stand)
	//				animationComponent->set_animation(model->stateAnimations[ModelStateAnimationStand].get());
	//			else if (state == Controller::State::forward)
	//				animationComponent->set_animation(model->stateAnimations[ModelStateAnimationForward].get());
	//		}
	//	}
	//}
}
