#pragma once

#include <vector>
#include <string>
#include <memory>

#include <flame/exports.h>

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

		FLAME_EXPORTS void add_alpha_channel();

		FLAME_EXPORTS void swap_RB();

		FLAME_EXPORTS void copy_to(Image *dst, int src_x, int src_y, int cx, int cy, int dst_x, int dst_y);
	};

	FLAME_EXPORTS Image *create_image(int cx, int cy, int channel, int bpp);

	FLAME_EXPORTS Image *load_image(const std::string &filename);

	FLAME_EXPORTS void save_image(int cx, int cy, int channel, int bpp, unsigned char *data, const std::string &filename);

	inline void save_image(Image *i, const std::string &filename)
	{
		save_image(i->cx, i->cy, i->channel, i->bpp, i->data, filename);
	}

	FLAME_EXPORTS void release_image(Image *i);
}
