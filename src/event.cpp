#include "event.h"
#include "core.h"

namespace tke
{
	std::vector<EventList*> eventLists;

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
			auto _t = nowTime - e.startTime;
			if (e.tickFunc) e.tickFunc(_t);

			if (_t >= e.duration)
			{
				if (e.execFunc) e.execFunc();
				list->currentEventIndex++;
				if (list->currentEventIndex < list->events.size())
					list->events[list->currentEventIndex].startTime = nowTime;
			}

			it++;
		}
	}
}