#include "../global.h"
#include "node.h"
#include "controller.h"

namespace tke
{
	ControllerComponent::ControllerComponent() :
		Component(ComponentTypeController)
	{
	}

	ControllerComponent::State ControllerComponent::get_state() const
	{
		return state;
	}

	float ControllerComponent::get_ang_offset() const
	{
		return ang_offset;
	}

	float ControllerComponent::get_speed() const
	{
		return speed;
	}

	float ControllerComponent::get_turn_speed() const
	{
		return turn_speed;
	}

	bool ControllerComponent::set_state(State _s, bool enable)
	{
		if (_s == State::StateStand)
		{
			if (state != State::StateStand)
			{
				state = State::StateStand;
				return true;
			}
			return false;
		}

		if (enable)
		{
			if (!(state & _s))
			{
				state = State(state | _s);
				return true;
			}
			return false;
		}
		else
		{
			if (state & _s)
			{
				state = State(state & ~_s);
				return true;
			}
			return false;
		}
	}

	void ControllerComponent::set_ang_offset(float v)
	{
		ang_offset = v;
	}

	void ControllerComponent::set_speed(float v)
	{
		speed = v;
	}

	void ControllerComponent::set_turn_speed(float v)
	{
		turn_speed = v;
	}

	void ControllerComponent::reset()
	{
		state = State::StateStand;
	}

	void ControllerComponent::on_update()
	{
		auto coord = get_parent()->get_coord();
		auto yaw = get_parent()->get_euler().x;

		if (state == StateStand)
			return;

		if (turn_speed > 0.f)
		{
			if ((state & StateTurnLeft) && !(state & StateTurnRight))
				yaw += turn_speed * elapsed_time;
			if ((state & StateTurnRight) && !(state & StateTurnLeft))
				yaw -= turn_speed * elapsed_time;
		}

		auto rad = glm::radians(yaw + ang_offset);

		if (speed > 0.f)
		{
			if ((state & StateForward) && !(state & StateBackward))
			{
				coord.x -= sin(rad) * speed * elapsed_time;
				coord.z -= cos(rad) * speed * elapsed_time;
			}
			if ((state & StateBackward) && !(state & StateForward))
			{
				coord.x += sin(rad) * speed * elapsed_time;
				coord.z += cos(rad) * speed * elapsed_time;
			}
			if ((state & StateLeft) && !(state & StateRight))
			{
				coord.x -= cos(rad) * speed * elapsed_time;
				coord.z += sin(rad) * speed * elapsed_time;
			}
			if ((state & StateRight) && !(state & StateLeft))
			{
				coord.x += cos(rad) * speed * elapsed_time;
				coord.z -= sin(rad) * speed * elapsed_time;
			}
			if ((state & StateUp) && !(state & StateDown))
				coord.y += speed * elapsed_time;
			if ((state & StateDown) && !(state & StateUp))
				coord.y -= speed * elapsed_time;
		}

		get_parent()->set_coord(coord);
		get_parent()->set_euler_x(yaw);
	}
}