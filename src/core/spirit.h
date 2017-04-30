#ifndef __TKE_SPIRIT__
#define __TKE_SPIRIT__

#include "math.h"

namespace tke
{
	struct Spirit
	{
		glm::vec2 coord;
		glm::vec2 size;
		unsigned int tex;
		glm::vec2 velocity;
		float mass;

		void show();
	};
}

#endif