#include <string>
#include <functional>
#include <memory>
#include <vector>

#include "../src/math/math.h"

int main(int argc, char** argv)
{
	union map_index
	{
		struct
		{
			unsigned char x;
			unsigned char y;
			unsigned char z;
			unsigned char w;
		};
		unsigned int packed;
	};

	map_index i;
	i.x = 1;
	i.y = 0;
	i.z = 0;
	i.w = 0;

	return 0;
}
