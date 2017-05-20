#include "object.h"

namespace tke
{
	Object::Object()
	{
		baseForwardAng = -90.f;
	}

	Object::~Object()
	{
		delete animationSolver;
		delete[]rigidDatas;
	}

	void Object::getRefrence()
	{
		refrenceCount++;
	}

	void Object::release()
	{
		refrenceCount--;
		if (refrenceCount == 0)
			delete this;
	}
}