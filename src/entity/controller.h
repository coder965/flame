#pragma once

#include "../math/math.h"

namespace tke
{
	struct Controller
	{
		enum class State
		{
			stand = 0,
			forward = 1 << 0,
			backward = 1 << 1,
			left = 1 << 2,
			right = 1 << 3,
			up = 1 << 4,
			down = 1 << 5,
			turn_left = 1 << 6,
			turn_right = 1 << 7,
			turn_up = 1 << 8,
			turn_down = 1 << 9
		};

		int lastTime = 0;
		float ang_offset = 0.f;
		float speed = 1.f;
		float turn_speed = 75.f;
		State state = State::stand;

		bool setState(State _s, bool enable);
		void reset();
		bool move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler);
	};
}