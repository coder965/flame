#include "_object.h"

namespace tke
{
	_Object::~_Object()
	{
		for (auto f : followings)
			f->remove_follower(this);
	}

	void _Object::follow_to(_Object *o)
	{
		followings.emplace_back(o);
		o->followers.emplace_back(this);
	}

	void _Object::remove_follower(_Object *o)
	{
		for (auto it = followers.begin(); it != followers.end(); it++)
		{
			if (*it == o)
			{
				followers.erase(it);
				break;
			}
		}
	}

	bool _Object::broadcast(_Object *o, Message msg)
	{
		for (auto f : followers)
		{
			if (f->on_message(o, msg))
				return true;
		}
		return false;
	}
}
