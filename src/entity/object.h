#pragma once

#include "../model/model.h"
#include "controller.h"

namespace tke
{
	REFLECTABLE enum class ObjectPhysicsType
	{
		REFLe null,
		REFLe static_r = 1 << 0, // cannot use with dynamic bit
		REFLe dynamic = 1 << 1, // cannot use with static bit
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

	REFLECTABLE struct Object : Transformer, Controller, Observed
	{
		REFL_BANK;

		REFLv std::string model_filename;
		Model *model = nullptr;

		REFLv std::string name;

		REFLe ObjectPhysicsType physics_type = ObjectPhysicsType::null; // cannot change

		std::unique_ptr<AnimationComponent> animationComponent;
		std::vector<ObjectRigidBodyData> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		int sceneIndex = -1;

		Object();
		Object(Model *_model, ObjectPhysicsType _physicsType = ObjectPhysicsType::null);
		~Object();
		void setState(Controller::State _s, bool enable);
	};
}
