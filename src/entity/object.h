#pragma once

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

	enum class ObjectPhysicsType
	{
		enable = 1 << 0,
		dynamic = 1 << 1,
		controller = 1 << 2
	};

	struct ObjectRigidBodyData
	{
		Rigidbody *rigidbody;
		physx::PxRigidActor *actor = nullptr;
		glm::mat3 rotation = glm::mat3(1.f);
		glm::vec3 coord = glm::vec3(0.f);

		~ObjectRigidBodyData();
	};

	struct Object : Node, Controller
	{
		std::string model_filename;
		std::shared_ptr<Model> model;

		std::string name;

		std::uint32_t physics_type = 0; // cannot change

		std::unique_ptr<AnimationRunner> animationComponent;
		std::vector<std::unique_ptr<ObjectRigidBodyData>> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		Object();
		Object(std::shared_ptr<Model> _model, unsigned int _physicsType = 0);
		virtual ~Object() override;
		void setState(Controller::State _s, bool enable);
	};
}
