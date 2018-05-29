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

#include <flame/filesystem.h>
#include <flame/system.h>

#include <Windows.h>

using namespace flame;

std::vector<std::string> include_dirs;
std::string link_libraries;
std::string input_dir;
std::string output_dir;

void compile(const char *cpp_filename)
{
	std::string cl("cl ");
	cl += cpp_filename;
	cl += " -LD -MD";
	cl += " -I \"C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\ucrt\"";
	cl += " -I \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\include\"";
	for (auto &d : include_dirs)
		cl += " -I " + d;
	cl += " -link"
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\um\\x64\\kernel32.lib\""
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\ucrt\\x64\\ucrt.lib\""
		" \"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17134.0\\um\\x64\\uuid.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\msvcprt.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\msvcrt.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\vcruntime.lib\""
		" \"D:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\lib\\x64\\oldnames.lib\""
		+ std::string(" ") + link_libraries;

	std::string out_parent_path;
	if (!output_dir.empty())
	{
		out_parent_path = output_dir;
		if (!is_slash(output_dir[output_dir.size() - 1]))
			out_parent_path += '/';
	}

	std::filesystem::path cpp_path(cpp_filename);

	cl += " -out:" +
		out_parent_path + cpp_path.filename().string() + ".dll";

	LongString output;
	exec("", cl.c_str(), &output);

	printf(output.data);

	return;
}

int main(int argc, char **args)
{
	for (auto i = 1; i < argc; )
	{
		std::string arg(args[i]);
		if (arg == "-i")
		{
			i++;
			for (; i < argc; )
			{
				std::string dir(args[i]);
				if (dir[0] == '-')
					break;
				include_dirs.push_back(dir);
				i++;
			}
		}
		else if (arg == "-l")
		{
			i++;
			for (; i < argc; )
			{
				std::string dir(args[i]);
				if (dir[0] == '-')
					break;
				link_libraries += dir + " ";
				i++;
			}
		}
		else if (arg == "-d")
		{
			i++;
			if (i < argc)
			{
				std::string dir(args[i]);
				if (dir[0] == '-')
					break;
				input_dir = dir;
				i++;
			}
		}
		else if (arg == "-o")
		{
			i++;
			if (i < argc)
			{
				std::string dir(args[i]);
				if (dir[0] == '-')
					break;
				output_dir = dir;
				i++;
			}
		}
		else
			i++;
	}

	compile(input_dir.c_str());

	system("pause");

	return 0;
}
