#include "object.h"

namespace tke
{
	Object::Object(ObjectType _type, Model *_pModel)
		:type(_type), pModel(_pModel)
	{
		if (pModel->rigidbodies.size() > 0)
			rigidDatas = new RigidData[pModel->rigidbodies.size()];
	}

	Object::~Object()
	{
		delete[]rigidDatas;
	}

	StaticObject::StaticObject(Model *_pModel)
		:Object(ObjectTypeStatic, _pModel)
	{
	}

	StaticObject::~StaticObject() {}

	AnimatedObject::AnimatedObject(Model *_pModel)
		: Object(ObjectTypeAnimated, _pModel)
	{
		animationSolver = new AnimationSolver(pModel);
	}

	AnimatedObject::~AnimatedObject() 
	{
		delete animationSolver;
	}
}