//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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

namespace flame
{
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;

	struct IVEC2
	{
		int x;
		int y;
	};

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
