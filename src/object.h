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
		ObjectPhysicsTypeNull,
		ObjectPhysicsTypeStatic,
		ObjectPhysicsTypeDynamic,
		ObjectPhysicsTypeController
	};

	struct RigidBodyData
	{
		Rigidbody *rigidbody;
		physx::PxRigidActor *actor;
		glm::mat3 rotation;
		glm::vec3 coord;
	};

	struct Model;
	struct Object : Transformer, Controller
	{
		Model *model = nullptr;

		AnimationComponent *animationComponent = nullptr;
		ObjectPhysicsType physicsType = ObjectPhysicsTypeNull;
		std::vector<RigidBodyData> rigidbodyDatas;

		int sceneIndex = -1;

		Object(Model *_model);
		~Object();
	};
}

#endif