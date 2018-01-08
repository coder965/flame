#pragma once

#if defined(_WIN64)
typedef __int64 TK_LONG_PTR;
#else
typedef _W64 long TK_LONG_PTR;
#endif
#define TK_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define TK_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define TK_LOW(I) ((I) & 0xffff)
#define TK_HIGH(I) ((I) >> 16)
#define TK_MAKEINT(H, L) ((L) | ((H) << 16))

template<size_t s> struct Sizer {};

namespace tke
{
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
}
