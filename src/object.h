#pragma once

#include <string>
#include <vector>

namespace tke
{
	enum Message
	{
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
		std::vector<Object*> followings;
		std::vector<Object*> followers;

		~Object();
		void follow_to(Object *o);
		void remove_following(Object *o);
		void remove_follower(Object *o);
		bool broadcast(Object *o, Message msg);
		virtual bool on_message(Object *sender, Message msg) { return false; };
	};
}
