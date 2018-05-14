#include "../../../src/flame/UI/UI.h"

namespace flame
{
	namespace UI
	{
		void show(void *i)
		{
			auto ui = (Instance*)i;
			ui->begin_window("Hey");
			static ValueInterpolater show = {0.f, 0.f, 2.f, 0.f, 2.f};
			show.step(ui->elapsed_time);
			if (show.v > 1.f)
				ui->text("hello");
			ui->end_window();
		}
	}
}

extern "C" {
	_declspec(dllexport) void show(void *i)
	{
		flame::UI::show(i);
	}
}
