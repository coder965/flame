#pragma once

#include <string>
#include <functional>

#if defined(_WIN64)
typedef long long TK_LONG_PTR;
typedef unsigned long long TK_ULONG_PTR;
#else
typedef long TK_LONG_PTR;
typedef unsigned long TK_ULONG_PTR;
#endif

#define TK_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define TK_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define TK_LOW(I) ((I) & 0xffff)
#define TK_HIGH(I) ((I) >> 16)
#define TK_MAKEINT(H, L) ((L) | ((H) << 16))
#define TK_INIT_BEGINE(n) struct n##_init{n##_init(){
#define TK_INIT_END }};

template<size_t s> struct Sizer {};

typedef void(*PF_EVENT0)();
typedef void(*PF_EVENT1)(int);
typedef void(*PF_EVENT2)(int, int);

template<typename T, typename... U>
size_t TK_GET_ADDRESS(std::function<T(U...)> f)
{
	typedef T(fnType)(U...);
	fnType ** fnPointer = f.template target<fnType*>();
	return (size_t)*fnPointer;
}

namespace flame
{
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;

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

	inline std::string get_error_string(Err errNum)
	{
		switch (errNum)
		{
			case NoErr:
				return "No error.";
			case ErrInvalidEnum:
				return "Invalid enum.";
			case ErrInvalidValue:
				return "Invalid value.";
			case ErrInvalidOperation:
				return "Invalid operation.";
			case ErrOutOfMemory:
				return "Out of memory.";
			case ErrContextLost:
				return "Context lost.";
			case ErrResourceLost:
				return "Resource lost.";
			default:
				return "unknow error";
		}
	}

	enum Op
	{
		OpNeedRemove,
		OpKeep,
		OpNeedUpdate
	};
}
