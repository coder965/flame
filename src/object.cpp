#include "object.h"

namespace tke
{
	Object::~Object()
	{
		for (auto f : followers)
			f->remove_following(this);
		for (auto f : followings)
			f->remove_follower(this);
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

	bool Object::broadcast(Object *o, Message msg, bool once)
	{
		for (auto f : followers)
		{
			if (f->on_message(o, msg) && once)
				return true;
		}
		return false;
	}

	void Object::add_deferred_message(Message msg, bool once)
	{
		deferred_messages.emplace_back(msg, once);
	}

	void link(Object *host, Object *guest)
	{
		guest->follow_to(host);
	}

	void break_link(Object *host, Object *guest)
	{
		host->remove_follower(guest);
		guest->remove_following(host);
	}
}
