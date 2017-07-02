#include "event.h"
#include "core.h"

namespace tke
{
	thread_local std::vector<EventList*> eventLists;

	void addEventList(EventList *p)
	{
		eventLists.push_back(p);
	}

	void removeEventList(EventList *p)
	{
		for (auto it = eventLists.begin(); it != eventLists.end(); it++)
		{
			if (*it == p)
			{
				eventLists.erase(it);
				return;
			}
		}
	}

	void processEvents()
	{
		for (auto it = eventLists.begin(); it != eventLists.end(); )
		{
			auto list = *it;

			if (list->currentEventIndex >= list->events.size())
			{
				if (list->repeat)
				{
					list->currentEventIndex = 0;
				}
				else
				{
					delete list;
					it = eventLists.erase(it);
					continue;
				}
				it++;
				continue;
			}

			Event &e = list->events[list->currentEventIndex];
			e.currentTime += timeDisp;
			if (e.tickFunc) e.tickFunc(e.currentTime);

			if (e.currentTime >= e.duration)
			{
				if (e.execFunc) e.execFunc();
				list->currentEventIndex++;
			}

			it++;
		}
	}
}