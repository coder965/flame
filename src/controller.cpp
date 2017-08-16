#include "controller.h"
#include "core.h"

namespace tke
{
	bool Controller::setState(State _s, bool enable)
	{
		if (_s == State::stand)
		{
			if (state != State::stand)
			{
				state = State::stand;
				return true;
			}
			return false;
		}

		if (enable)
		{
			if (!((int)state & (int)_s))
			{
				state = State((int)state | (int)_s);
				return true;
			}
			return false;
		}
		else
		{
			if ((int)state & (int)_s)
			{
				state = State((int)state & ~(int)_s);
				return true;
			}
			return false;
		}
	}

	void Controller::reset()
	{
		state = State::stand;
		lastTime = nowTime;
	}

	bool Controller::move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler)
	{
		outCoord = glm::vec3();
		outEuler = glm::vec3();

		float dist = (nowTime - lastTime) / 1000.f;
		lastTime = nowTime;

		if (state == State::stand)
			return false;

		inEulerX = glm::radians(inEulerX + ang_offset);

		bool changed = false;

		if (speed > 0.f)
		{
			if (((int)state & (int)State::forward) && !((int)state & (int)State::backward))
			{
				outCoord.x -= sin(inEulerX) * speed * dist;
				outCoord.z -= cos(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::backward) && !((int)state & (int)State::forward))
			{
				outCoord.x += sin(inEulerX) * speed * dist;
				outCoord.z += cos(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::left) && !((int)state & (int)State::right))
			{
				outCoord.x -= cos(inEulerX) * speed * dist;
				outCoord.z += sin(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::right) && !((int)state & (int)State::left))
			{
				outCoord.x += cos(inEulerX) * speed * dist;
				outCoord.z -= sin(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::up) && !((int)state & (int)State::down))
			{
				outCoord.y += speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::down) && !((int)state & (int)State::up))
			{
				outCoord.y -= speed * dist;
				changed = true;
			}
		}

		if (turn_speed > 0.f)
		{
			if (((int)state & (int)State::turn_left) && !((int)state & (int)State::turn_right))
			{
				outEuler.x += turn_speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::turn_right) && !((int)state & (int)State::turn_left))
			{
				outEuler.x -= turn_speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::turn_up) && !((int)state & (int)State::turn_down))
			{
				outEuler.z += turn_speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::turn_down) && !((int)state & (int)State::turn_up))
			{
				outEuler.z -= turn_speed * dist;
				changed = true;
			}
		}

		return changed;
	}

}