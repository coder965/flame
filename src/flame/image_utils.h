#pragma once

#include <vector>
#include <string>
#include <memory>

#include "math/math.h"

namespace tke
{
	int calc_pitch(int cx);
	int calc_pitch(int cx, int bpp);

	struct Image
	{
		int cx;
		int cy;
		int channel;
		int bpp;
		int level;
		int layer;
		bool sRGB;
		std::unique_ptr<unsigned char[]> data;

		struct LevelCache
		{
			int cx;
			int cy;
			int pitch;
			int data_offset;
			int size;
		};

		std::vector<LevelCache> level_cache;

		Image(const std::string &filename);
		Image(int _cx, int _cy, int _channel, int _bpp, int _level, int _layer, unsigned char *_data = nullptr);
		void clear(float color);
		void clear(glm::vec2 color);
		void clear(glm::vec3 color);
		void clear(glm::vec4 color);
		int get_cx(int _level = 0) const;
		int get_cy(int _level = 0) const;
		unsigned char *get_data(int _level = 0) const;
		void save(const std::string &filename);

		Image *create_distance_transform(int offset);
	};
}
