//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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
			if (ext == ".h" || ext == ".c" || ext == ".cpp" || ext == ".hpp" ||
				ext == ".glsl" || ext == ".vert" || ext == ".tesc" || ext == ".tese" || ext == ".geom" || ext == ".frag")
			{
				auto f = std::ifstream(p.string());
				std::string line;
				while (!f.eof())
				{
					total_lines++;
					std::getline(f, line);
					for (auto &chr : line)
					{
						if (chr != ' ' && chr != 9 && chr != '\r')
						{
							SLOC++;
							break;
						}
					}
				}
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
