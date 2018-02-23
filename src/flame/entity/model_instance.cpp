//#include "../graphics/buffer.h"
//#include "../model/animation.h"
//#include "../physics/physics.h"
#include <flame/entity/model.h>
#include <flame/utils/filesystem.h>
#include "model_instance.h"

namespace tke
{
	void ModelInstanceComponent::serialize(XMLNode *dst)
	{
		dst->add_attribute(new XMLAttribute("model", model->filename));
	}

	void ModelInstanceComponent::unserialize(XMLNode *dst)
	{
		auto m = getModel(dst->first_attribute("model")->get_string());
		if (m)
			model = m;
		else
			model = cubeModel;
	}

	ModelInstanceComponent::ModelInstanceComponent() :
		Component(ComponentTypeModelInstance),
		model(cubeModel),
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

	void ModelInstanceComponent::set_model(std::shared_ptr<Model> _model)
	{
		model = _model;

		broadcast(this, MessageChangeModel);
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
