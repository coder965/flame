#include <sstream>
#include <stack>
#include <regex>

#include <flame/global.h>
#include <flame/filesystem.h>
#include <flame/system.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/shader.h>
#include <flame/engine/graphics/pipeline.h>

namespace flame
{
	static std::vector<std::weak_ptr<Shader>> _shaders;

	std::shared_ptr<Shader> get_shader(const std::string &filename, const std::vector<std::string> &defines, Pipeline *pipeline)
	{
		for (auto it = _shaders.begin(); it != _shaders.end(); )
		{
			auto s = it->lock();
			if (s)
			{
				if (s->filename == filename && s->defines == defines)
				{
					if (pipeline)
						s->referencing_pipelines.push_back(pipeline);
					return s;
				}
				it++;
			}
			else
				it = _shaders.erase(it);
		}
		auto s = std::make_shared<Shader>(filename, defines);
		if (pipeline)
			s->referencing_pipelines.push_back(pipeline);
		_shaders.push_back(s);
		return s;
	}

	FileWatcher *shader_change_watcher = nullptr;

	void setup_shader_file_watcher()
	{
		static bool first = true;
		if (first)
		{
			first = false;

			shader_change_watcher = add_file_watcher(FileWatcherModeContent, shader_path + "src/", [](const std::vector<FileChangeInfo> &infos) {
				std::vector<Shader*> changed_shaders;
				for (auto &i : infos)
				{
					Shader *shader = nullptr;
					for (auto &t : _shaders)
					{
						auto s = t.lock();
						if (s && s->filename == i.filename)
						{
							shader = s.get();
							break;
						}
					}
					if (!shader)
						continue;
					auto exist = false;
					for (auto &t : changed_shaders)
					{
						if (t == shader)
						{
							exist = true;
							break;
						}
					}
					if (!exist)
						changed_shaders.push_back(shader);
				}
				add_after_frame_event([=]() {
					for (auto s : changed_shaders)
						s->create();
				});
			});
		}
	}
}
