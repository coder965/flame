#pragma once

#include "transformer.h"
#include "controller.h"
#include "node.h"

namespace physx
{
	struct PxRigidActor;
	struct PxController;
}

namespace tke
{
	struct Model;
	struct AnimationRunner;
	struct Rigidbody;

	REFLECTABLE enum class ObjectPhysicsType
	{
		REFLe enable = 1 << 0,
		REFLe dynamic = 1 << 1,
		REFLe controller = 1 << 2
	};

	struct ObjectRigidBodyData
	{
		Rigidbody *rigidbody;
		physx::PxRigidActor *actor = nullptr;
		glm::mat3 rotation = glm::mat3(1.f);
		glm::vec3 coord = glm::vec3(0.f);

		~ObjectRigidBodyData();
	};

	REFLECTABLE struct Object : Transformer, Controller, Node
	{
		REFL_BANK;

		REFLv std::string model_filename;
		std::shared_ptr<Model> model;

		REFLv std::string name;

		REFLv std::uint32_t physics_type = 0; // cannot change

		std::unique_ptr<AnimationRunner> animationComponent;
		std::vector<std::unique_ptr<ObjectRigidBodyData>> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		Object();
		Object(std::shared_ptr<Model> _model, unsigned int _physicsType = 0);
		~Object();
		void setState(Controller::State _s, bool enable);
	};
}
