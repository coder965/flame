#ifndef __TKE_OBJECT__
#define __TKE_OBJECT__

#include "transformer.h"
#include "controler.h"
#include "model.h"
#include "physics.h"

namespace tke
{
	enum ObjectPhysicsType
	{
		ObjectPhysicsTypeNull = 0,
		ObjectPhysicsTypeStatic = 1 << 0, // cannot use with dynamic bit
		ObjectPhysicsTypeDynamic = 1 << 1, // cannot use with static bit
		ObjectPhysicsTypeController = 1 << 2
	};

	struct RigidBodyData
	{
		Rigidbody *rigidbody;
		physx::PxRigidActor *actor;
		glm::mat3 rotation;
		glm::vec3 coord;
	};

	struct Model;
	struct Object : ObservedObject, Transformer, Controller
	{
		Model *model;

		ObjectPhysicsType physicsType; // cannot change

		AnimationComponent *animationComponent = nullptr;
		std::vector<RigidBodyData> rigidbodyDatas;
		physx::PxController *pxController = nullptr;
		float floatingTime = 0.f;

		int sceneIndex = -1;

		Object(Model *_model, ObjectPhysicsType _physicsType = ObjectPhysicsTypeNull);
		~Object();
	};
}

#endif