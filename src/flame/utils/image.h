#pragma once

#include <vector>
#include <string>
#include <memory>

#include <flame/math/math.h>

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
		bool sRGB;
		std::unique_ptr<unsigned char[]> data;

		Image(const std::string &filename);
		Image(int _cx, int _cy, int _channel, int _bpp, unsigned char *_data = nullptr);
		void clear(float color);
		void clear(glm::vec2 color);
		void clear(glm::vec3 color);
		void clear(glm::vec4 color);
		void save(const std::string &filename);

		Image *create_distance_transform(int offset);
	};
}
