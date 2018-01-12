#pragma once

#include "component.h"

//namespace physx
//{
//	struct PxRigidActor;
//	struct PxController;
//}

namespace tke
{
	struct Model;
	struct AnimationRunner;
	struct Rigidbody;

	//enum ModelInstanceComponentPhysicsType
	//{
	//	ModelInstanceComponentEnable = 1 << 0,
	//	ModelInstanceComponentDynamic = 1 << 1,
	//	ModelInstanceComponentController = 1 << 2
	//};

	//struct ModelInstanceComponentRigidBodyData
	//{
	//	Rigidbody *rigidbody;
	//	physx::PxRigidActor *actor = nullptr;
	//	glm::mat3 rotation = glm::mat3(1.f);
	//	glm::vec3 coord = glm::vec3(0.f);

	//	~ModelInstanceComponentRigidBodyData();
	//};

	class ModelInstanceComponent : Component
	{
	private:
		std::shared_ptr<Model> model;

		//std::uint32_t physics_type = 0;

		//std::unique_ptr<AnimationRunner> animationComponent;
		//std::vector<std::unique_ptr<ModelInstanceComponentRigidBodyData>> rigidbodyDatas;
		//physx::PxController *pxController = nullptr;
		//float floatingTime = 0.f;
	public:
		ModelInstanceComponent(std::shared_ptr<Model> _model);
	};
}
