#include <chrono>

#include "time.h"

namespace tke
{
	long long get_now_time_ms()
	{
		return std::chrono::system_clock::now().time_since_epoch().count() / 10000;
	}
}
