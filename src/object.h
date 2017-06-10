#ifndef __TKE_OBJECT__
#define __TKE_OBJECT__

#include "transformer.h"
#include "controler.h"
#include "model.h"

namespace tke
{
	enum ObjectType
	{
		ObjectTypeStatic,
		ObjectTypeAnimated
	};

	struct Model;
	struct Object : Transformer, Controller
	{
		ObjectType type;

		Model *pModel = nullptr;
		bool phyx = false;

		float floatingTime = 0.f;
		RigidData *rigidDatas = nullptr;

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
		float frame = 0;
		BoneData *boneData = nullptr;
		glm::mat4 *boneMatrix = nullptr;
		UniformBuffer *boneMatrixBuffer = nullptr;

		AnimatedObject(Model *_pModel);
		virtual ~AnimatedObject() override;
		void setAnimation(Animation *animation);
		void sample();
		void calcIK();
		void fixMatrix();
		void updateUBO();
	};
}

#endif