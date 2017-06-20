#pragma once

#include <vector>

namespace tke
{
	typedef void(*PF_TICK)(int);
	typedef void(*PF_EXEC)();

	struct Event
	{
		PF_TICK tickFunc = nullptr;
		int duration = 1;
		PF_EXEC execFunc = nullptr;

		int startTime = 0;
	};

	struct EventList
	{
		std::vector<Event> events;
		bool repeat = false;
		int currentEventIndex = 0;
	};

	void addEventList(EventList *);
	void removeEventList(EventList *);
	void processEvents();
}
