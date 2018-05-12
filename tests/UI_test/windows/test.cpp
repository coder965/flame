#include <flame/UI/UI.h>

namespace flame
{
	namespace UI
	{
		void say_hello(void *i)
		{
			auto ui = (Instance*)i;
			ui->text("hello");
		}
	}
}

extern "C" {
	_declspec(dllexport) void say_hello(void *i)
	{
		flame::UI::say_hello(i);
	}
}
