#pragma once

#include <string>
#include <vector>

namespace tke
{
	enum Message
	{
		MessageWaterAdd,
		MessageWaterRemove
	};

	struct _Object
	{
		std::string name;
		std::vector<_Object*> followings;
		std::vector<_Object*> followers;

		~_Object();
		void follow_to(_Object *o);
		void remove_follower(_Object *o);
		bool broadcast(_Object *o, Message msg);
		virtual bool on_message(_Object *sender, Message msg) { return false; };
	};
}
