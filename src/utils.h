#pragma once

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <chrono>

template<size_t s> struct Sizer {};

namespace tke
{
	typedef void(*PF_EVENT0)();
	typedef void(*PF_EVENT1)(int);
	typedef void(*PF_EVENT2)(int, int);

	inline long long now_time_ms()
	{
		return std::chrono::system_clock::now().time_since_epoch().count() / 10000;
	}

	int lineNumber(const char *str);

	enum Err
	{
		NoErr,
		ErrInvalidEnum,
		ErrInvalidValue,
		ErrInvalidOperation,
		ErrOutOfMemory,
		ErrContextLost,
		ErrResourceLost
	};

	const char *getErrorString(Err errNum);

	void *get_hinst();
	int get_screen_cx();
	int get_screen_cy();
	std::string get_exe_path();

	struct KeyState
	{
		bool justDown = false;
		bool justUp = false;
		bool pressing = false;
	};

	const char *getClipBoard();
	void setClipBoard(const std::string &);

	std::string translate(int srcCP, int dstCP, const std::string &src);
	std::string japaneseToChinese(const std::string &src);

	void exec(const std::string &filename, const std::string &parameters);
}
