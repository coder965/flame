
#include <experimental/filesystem>

int main(int argc, char **args)
{
	auto s = "a.AbC";
	std::experimental::filesystem::path p(s);
	auto e = p.extension().string();


	return 0;
}
