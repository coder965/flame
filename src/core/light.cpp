#include "light.h"

namespace tke
{
	void Light::getRefrence()
	{
		refrenceCount++;
	}

	void Light::release()
	{
		refrenceCount--;
		if (refrenceCount == 0)
			delete this;
	}

	char *typeNames[] = {
		"parallax light",
		"point light"
	};

	char *Light::getTypeName(Type _type)
	{
		return typeNames[(int)_type];
	}

	char *Light::getTypeName()
	{
		return typeNames[(int)type];
	}
}
