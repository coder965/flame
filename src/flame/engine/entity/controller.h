#pragma once

#include <flame/common/math.h>
#include <flame/engine/entity/component.h>

namespace flame
{
	class ControllerComponent : public Component
	{
	public:
		enum State
		{
			StateStand = 0,
			StateForward = 1 << 0,
			StateBackward = 1 << 1,
			StateLeft = 1 << 2,
			StateRight = 1 << 3,
			StateUp = 1 << 4,
			StateDown = 1 << 5,
			StateTurnLeft = 1 << 6,
			StateTurnRight = 1 << 7
		};
	private:
		State state = State::StateStand;
		float ang_offset = 0.f;
		float speed = 1.f;
		float turn_speed = 75.f;
	public:
		ControllerComponent();
		
		State get_state() const;
		float get_ang_offset() const;
		float get_speed() const;
		float get_turn_speed() const;

		bool set_state(State _s, bool enable);
		void set_ang_offset(float v);
		void set_speed(float v);
		void set_turn_speed(float v);

		void reset();
	protected:
		virtual void on_update() override;
	};
}
