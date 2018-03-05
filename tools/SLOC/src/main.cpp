#include <vector>
#include <filesystem>
#include <fstream>
#include <iterator>

#include <flame/common/filesystem.h>

namespace fs = std::experimental::filesystem;

static std::vector<std::string> excludes;

static long long total_lines = 0;
static long long SLOC = 0;

void calc(const fs::path &path)
{
	if (!fs::exists(path))
		return;
	for (auto &e : excludes)
	{
		if (e == path.string())
			return;
	}

	fs::directory_iterator end_it;
	for (fs::directory_iterator it(path); it != end_it; it++)
	{
		if (fs::is_directory(it->status()))
		{
			auto p = it->path();
			auto s = p.string();
			if (s != "." && s != "..")
				calc(p);
		}
		else
		{
			auto p = it->path();
			auto ext = p.extension().string();
			if (ext == ".h" || ext == ".c" || ext == ".cpp" || ext == ".hpp")
			{
				auto f = flame::get_file_content(p.string());
				auto last_line_has_content = false;
				auto fun_judge = [&]() {
					total_lines++;
					if (last_line_has_content)
						SLOC++;
				};
				for (auto i = 0; i < f.second; i++)
				{
					auto chr = f.first[i];
					if (chr == '\n')
					{
						fun_judge();
						last_line_has_content = false;
					}
					else if (chr != ' ' && chr != 9 && chr != '\r')
						last_line_has_content = true;
				}
				fun_judge();
			}
		}
	}
}

int main(int argc, char **args)
{
	if (argc < 2)
		return 0;

	for (auto i = 2; i < argc; i++)
	{
		if (args[i] == std::string("-e"))
		{
			for (auto j = i + 1; j < argc; j++)
			{
				if (args[j][0] != '-')
				{
					excludes.emplace_back(args[j]);
					i++;
				}
				else
					break;
			}
		}
	}

	calc(args[1]);

	printf("total:%d\n", total_lines);
	printf("SLOC:%d\n", SLOC);

	return 0;
}
