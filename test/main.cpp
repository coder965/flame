#include <string>
#include <functional>
#include <memory>
#include <vector>

#include "../src/math/math.h"

int main(int argc, char** argv)
{
	auto m = glm::translate(glm::vec3(1, 2, 3));
	auto v = glm::vec3(m[3]);

	return 0;
}
