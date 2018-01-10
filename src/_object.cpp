#include "_object.h"

namespace tke
{
	_Object::~_Object()
	{
		for (auto f : followings)
			f->remove_listener(this);
	}

	void _Object::listen_to(_Object *o)
	{
		followings.emplace_back(o);
		o->followers.emplace_back(this);
	}

	void _Object::remove_listener(_Object *o)
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

	void _Object::broadcast(Message msg)
	{
		for (auto f : followers)
			f->on_message(this, msg);
	}
}
