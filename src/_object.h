#pragma once

#include <string>
#include <vector>

namespace tke
{
	enum Message
	{

	};

	struct _Object
	{
		std::string name;
		std::vector<_Object*> followings;
		std::vector<_Object*> followers;

		virtual ~_Object();
		void listen_to(_Object *o);
		void remove_listener(_Object *o);
		void broadcast(Message msg);
		virtual void on_message(_Object *sender, Message msg) = 0;
	};
}
