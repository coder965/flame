#include <string>
#include <functional>
#include <memory>
#include <vector>

#include "../src/math/math.h"

int main(int argc, char** argv)
{
	auto proj = glm::perspective(60.f, 1.3f, 0.1f, 1000.f);

	auto p0 = proj * glm::vec4(1, 2, 3, 1);
	auto p1 = proj * glm::vec4(3, 4, 5, 1);
	auto pa = (p0 + p1) * 0.5f;
	auto p2 = glm::vec4(1, 2, 3, 1);
	auto p3 = glm::vec4(3, 4, 5, 1);
	auto pb = proj * ((p0 + p1) * 0.5f);

	return 0;
}
