#include <regex>

#include "../core/scene.h"
#include "../core/core.h"
#include "model.file.h"
#include "script.h"

namespace tke
{
	void processCmdLine(const std::string &str, bool record)
	{
		static std::string last_cmd;

		std::string string(str);

		std::regex pat(R"([\w\.]+)");
		std::smatch sm;

		if (std::regex_search(string, sm, pat))
		{
			if (sm[0].str() == "r")
			{
				processCmdLine(last_cmd.c_str(), false);
			}
			else if (sm[0].str() == "reload")
			{
				if (record) last_cmd = string;
				string = sm.suffix();
				if (std::regex_search(string, sm, pat))
				{
					//tk::Vk::deviceWaitIdle();
					scene->needUpdataSky = true;
					postRedrawRequest();
				}
			}
		}
	}
}