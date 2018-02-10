#pragma once

#include <string>
#include <list>

namespace tke
{
	enum Message
	{
		MessageResolutionChange,
		MessageNodeAdd,
		MessageNodeRemove,
		MessageComponentAdd,
		MessageComponentRemove,
		MessageSkyDirty,
		MessageAmbientDirty,
		MessageToggleShaodw,
		MessageChangeModel
	};

	struct Object
	{
		std::string name;
		std::list<Object*> followings;
		std::list<Object*> followers;

		~Object();
		void follow_to(Object *o);
		void remove_following(Object *o);
		void remove_follower(Object *o);
		bool broadcast(Object *o, Message msg, bool once = true);
		virtual bool on_message(Object *sender, Message msg) { return false; };
	};

	void link(Object *host, Object *guest);
	void break_link(Object *host, Object *guest);
}
