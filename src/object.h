#ifndef __TKE_OBJECT__
#define __TKE_OBJECT__

#include "transformer.h"
#include "controler.h"
#include "model.h"
#include "physics.h"

namespace tke
{
	enum ObjectType
	{
		ObjectTypeStatic,
		ObjectTypeAnimated
	};

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
		ObjectType type;

		Model *pModel = nullptr;

		ObjectPhysicsType physicsType = ObjectPhysicsTypeNull;
		std::vector<RigidBodyData> rigidbodyDatas;

		int sceneIndex = -1;

		Object(ObjectType _type, Model *_pModel);
		virtual ~Object();
	};

	struct StaticObject : Object
	{
		StaticObject(Model *_pModel);
		virtual ~StaticObject() override;
	};

	struct AnimatedObject : Object
	{
		Animation *currentAnimation = nullptr;
		float currentFrame = 0.f;
		float currentTime = 0.f;
		BoneData *boneData = nullptr;
		glm::mat4 *boneMatrix = nullptr;
		UniformBuffer *boneMatrixBuffer = nullptr;

		AnimatedObject(Model *_pModel);
		virtual ~AnimatedObject() override;
		void setAnimation(Animation *animation);
		void update();
	};
}

#endif