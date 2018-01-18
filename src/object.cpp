#include "object.h"

namespace tke
{
	Object::~Object()
	{
		for (auto f : followers)
			f->remove_following(this);
	}

	void Object::follow_to(Object *o)
	{
		followings.emplace_back(o);
		o->followers.emplace_back(this);
	}

	void Object::remove_following(Object *o)
	{
		for (auto it = followings.begin(); it != followings.end(); it++)
		{
			if (*it == o)
			{
				followings.erase(it);
				break;
			}
		}
	}

	void Object::remove_follower(Object *o)
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

	bool Object::broadcast(Object *o, Message msg)
	{
		for (auto f : followers)
		{
			if (f->on_message(o, msg))
				return true;
		}
		return false;
	}
}
