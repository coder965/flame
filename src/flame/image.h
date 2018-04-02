#pragma once

#include <flame/exports.h>

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

		void calc_size()
		{
			pitch = calc_pitch(cx, bpp);
			size = pitch * cy;
			data = new unsigned char[size];
		}

		void add_alpha_channel()
		{
			assert(channel == 3);
			auto new_data = new unsigned char[cx * cy * 4];
			pitch = cx * 4;
			for (auto j = 0; j < cy; j++)
			{
				for (auto i = 0; i < cx; i++)
				{
					new_data[j * pitch + i * 4 + 0] = data[j * cx * 3 + i * 3 + 0];
					new_data[j * pitch + i * 4 + 1] = data[j * cx * 3 + i * 3 + 1];
					new_data[j * pitch + i * 4 + 2] = data[j * cx * 3 + i * 3 + 2];
					new_data[j * pitch + i * 4 + 3] = 255;
				}
			}
			channel = 4;
			bpp = 32;
			size = cx * 4 * cy;
			delete[]data;
			data = new_data;
		}

		void swap_RB()
		{
			assert(channel > 2);
			for (auto j = 0; j < cy; j++)
			{
				for (auto i = 0; i < cx; i++)
					std::swap(data[j * pitch + i * channel + 0], data[j * pitch + i * channel + 2]);
			}
		}

		void copy_to(Image *dst, int src_x, int src_y, int cx, int cy, int dst_x, int dst_y)
		{
			for (auto j = 0; j < cy; j++)
			{
				for (auto i = 0; i < cx; i++)
					dst->data[(dst_y + j) * dst->pitch + dst_x + i] = data[(src_y + j) * pitch + src_x + i];
			}
		}
	};

	inline Image *create_image(int cx, int cy, int channel, int bpp)
	{
		auto i = new Image;
		i->cx = cx;
		i->cy = cy;
		i->channel = channel;
		i->bpp = bpp;
		i->calc_size();
		memset(i->data, 0, i->size);
		return i;
	}

	FLAME_EXPORTS Image *load_image(const std::string &filename);

	FLAME_EXPORTS void save_image(int cx, int cy, int channel, int bpp, unsigned char *data, const std::string &filename);

	inline void save_image(Image *i, const std::string &filename)
	{
		save_image(i->cx, i->cy, i->channel, i->bpp, i->data, filename);
	}

	FLAME_EXPORTS void release_image(Image *i);
}
