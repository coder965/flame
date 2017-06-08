#include "light.h"

namespace tke
{
	Light::Light(Type _type)
		:type(_type)
	{
	}

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
