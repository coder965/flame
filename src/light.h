#ifndef __TKE_LIGHT__
#define __TKE_LIGHT__

#include "math.h"
#include "transformer.h"

namespace tke
{
	enum LightType
	{
		LightTypeParallax,
		LightTypePoint,
		LightTypeSpot
	};

	struct Light : Transformer
	{
		LightType type;
		glm::vec3 color = glm::vec3(0.5f);

		bool shadow = false;

		int sceneIndex = -1;
		int sceneShadowIndex = -1;

		Light(LightType _type);
		static char *getTypeName(LightType _type);
		char *getTypeName();
	};

	struct ParallaxLight : Light
	{
		ParallaxLight();
	};

	struct PointLight : Light
	{
		glm::vec3 decayFactor = glm::vec3(0.f, 0.f, 1.f);

		PointLight();
	};

	struct SpotLight : PointLight
	{
		glm::vec3 spotDirection = glm::vec3(1.f, 0.f, 0.f);
		float spotRange = 0.5f;

		SpotLight();
	};
}

#endif