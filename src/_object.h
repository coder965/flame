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

		virtual ~_Object();
		void listen_to(_Object *o);
		void remove_listener(_Object *o);
		bool broadcast(_Object *o, Message msg);
		virtual bool on_message(_Object *sender, Message msg) { return false; };
	};
}
