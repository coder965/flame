#include "light.h"

namespace tke
{
	Light::Light(LightType _type)
		:type(_type)
	{
	}

	std::string getLightTypeName(LightType _type)
	{
		char *typeNames[] = {
			"parallax light",
			"point light",
			"spot light"
		};
		return typeNames[_type];
	}
}
