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

#ifdef _FLAME_IMAGE_EXPORTS
#define FLAME_IMAGE_EXPORTS __declspec(dllexport)
#else
#define FLAME_IMAGE_EXPORTS __declspec(dllimport)
#endif

#include <vector>
#include <string>
#include <memory>

namespace flame
{
	inline int calc_pitch(int cx)
	{
		auto pitch = cx;
		if (pitch % 4 == 0)
			return pitch;
		return pitch + 4 - pitch % 4;
	}

	inline int calc_pitch(int cx, int bpp)
	{
		return calc_pitch(cx * (bpp / 8));
	}

	struct Image
	{
		int cx;
		int cy;
		int channel;
		int bpp;
		bool sRGB;
		int pitch;
		int size;
		unsigned char *data;

		FLAME_IMAGE_EXPORTS void add_alpha_channel();

		FLAME_IMAGE_EXPORTS void swap_RB();

		FLAME_IMAGE_EXPORTS void copy_to(Image *dst, int src_x, int src_y, int cx, int cy, int dst_x, int dst_y);
	};

	FLAME_IMAGE_EXPORTS Image *create_image(int cx, int cy, int channel, int bpp);

	FLAME_IMAGE_EXPORTS Image *load_image(const std::string &filename);

	FLAME_IMAGE_EXPORTS void save_image(int cx, int cy, int channel, int bpp, unsigned char *data, const std::string &filename);

	inline void save_image(Image *i, const std::string &filename)
	{
		save_image(i->cx, i->cy, i->channel, i->bpp, i->data, filename);
	}

	FLAME_IMAGE_EXPORTS void release_image(Image *i);
}
