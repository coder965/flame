#include "light.h"

namespace tke
{
	Light::Light(LightType _type)
		:type(_type)
	{
	}

	char *typeNames[] = {
		"parallax light",
		"point light",
		"spot light"
	};

	char *Light::getTypeName(LightType _type)
	{
		return typeNames[_type];
	}

	char *Light::getTypeName()
	{
		return typeNames[type];
	}

	ParallaxLight::ParallaxLight()
		:Light(LightTypeParallax)
	{}

	PointLight::PointLight()
		: Light(LightTypePoint)
	{}

	SpotLight::SpotLight()
	{
		type = LightTypeSpot;
	}
}
