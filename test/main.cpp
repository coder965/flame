#include <string>

#include <functional>
#include <memory>

#include "../src/math/math.h"


int main(int argc, char** argv)
{
	auto s0 = sizeof(std::unique_ptr<int>);
	auto s1 = sizeof(std::shared_ptr<int>);
	auto s2 = sizeof(std::function<void()>);

	return 0;
}

