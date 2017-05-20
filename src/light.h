#ifndef __TKE_LIGHT__
#define __TKE_LIGHT__

#include "math.h"
#include "transformer.h"

namespace tke
{
	struct Light : Transformer
	{
		enum class Type : int
		{
			eParallax,
			ePoint
		};

		int refrenceCount = 0;
		bool dying = false;

		Type type;
		glm::vec3 color = glm::vec3(0.5f);
		glm::vec3 decayFactor = glm::vec3(0.f, 0.f, 1.f);

		bool shadow = false;

		int sceneIndex = -1;
		int sceneShadowIndex = -1;

		void getRefrence();
		void release();
		static char *getTypeName(Type _type);
		char *getTypeName();
	};
}

#endif