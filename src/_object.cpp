#include "_object.h"

namespace tke
{
	_Object::~_Object()
	{
		for (auto f : followings)
		{
			for (auto it = f->followers.begin(); it != f->followers.end(); it++)
			{
				if (*it == this)
				{
					f->followers.erase(it);
					break;
				}
			}
		}
	}

	void _Object::listen_to(_Object *o)
	{
		followings.emplace_back(o);
		o->followers.emplace_back(this);
	}

	void _Object::broadcast(Message msg)
	{
		for (auto f : followers)
			f->on_message(this, msg);
	}
}
