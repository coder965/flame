#pragma once

#include <flame/common_exports.h>

#include <vector>
#include <string>
#include <memory>

#include <flame/common/math.h>

namespace flame
{
	FLAME_COMMON_EXPORTS int calc_pitch(int cx);
	FLAME_COMMON_EXPORTS int calc_pitch(int cx, int bpp);

	FLAME_COMMON_EXPORTS struct Image
	{
		int cx;
		int cy;
		int channel;
		int bpp;
		int pitch;
		bool sRGB;
		unsigned char *data;
		bool own_data;

		Image(const std::string &filename);
		Image(int _cx, int _cy, int _channel, int _bpp, unsigned char *_data = nullptr, bool _own_data = true);
		~Image();
		void clear(float color);
		void clear(glm::vec2 color);
		void clear(glm::vec3 color);
		void clear(glm::vec4 color);
		void copy_to(Image *dst, int src_x, int src_y, int cx, int cy, int dst_x, int dst_y);
		void save(const std::string &filename);
		void save_as_raw_bit_rgba32(const std::string &filename);
	};
}
