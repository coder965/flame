struct Pipeline;

const std::string stageNames[] = {
	"vert",
	"tesc",
	"tese",
	"geom",
	"frag"
};

const int stageTypes[] = {
	(int)tke::StageFlags::vert,
	(int)tke::StageFlags::tesc,
	(int)tke::StageFlags::tese,
	(int)tke::StageFlags::geom,
	(int)tke::StageFlags::frag
};

struct Stage : tke::StageAbstract
{
	std::string text;
	std::string output;
	std::string compileOutput;

	struct Wrap : QObject
	{
		int type;

		bool changed = false;
		MyEdit *edit = nullptr;
		int tabIndex = -1;
		~Wrap() { delete edit; }
		void appear()
		{
			edit = new MyEdit;
			edit->setLineWrapMode(QPlainTextEdit::NoWrap);
			connect(edit, &QPlainTextEdit::textChanged, this, &Wrap::on_text_changed);
		}
		void setTitle()
		{
			for (int i = 0; i < 5; i++)
			{
				if (stageTypes[i] == type)
				{
					QString title = stageNames[(int)i].c_str();
					if (changed) title += "*";
					stageTab->setTabText(tabIndex, title);
					return;
				}
			}
		}
		void on_text_changed()
		{
			if (preparingData) return;

			changed = true;
			setTitle();
		}
	}wrap;

	void appear()
	{
		wrap.type = (int)type;
		wrap.appear();
		wrap.edit->setPlainText(text.c_str());
	}

	std::string Stage::getFullText(const std::string &parent_path)
	{
		std::stringstream strIn(text);
		std::string strOut;
		strOut += "#version 450 core\n";
		strOut += "#extension GL_ARB_separate_shader_objects : enable\n";
		strOut += "#extension GL_ARB_shading_language_420pack : enable\n";
		strOut += "\n";

		std::string line;
		while (!strIn.eof())
		{
			std::getline(strIn, line);

			std::regex pat(R"(#include\s+\"([\w\.\\]+)\")");
			std::smatch sm;
			if (std::regex_search(line, sm, pat))
			{
				auto include = sm[1].str();
				tke::OnceFileBuffer file(parent_path + "/" + filepath + "/" + include);
				strOut += file.data;
				strOut += "\n";
			}
			else
			{
				strOut += line + "\n";
			}
		}
		return strOut;
	}

	void on_save();
	void on_spv();
};

struct Pipeline : tke::PipelineAbstract<Stage>
{
	bool changed = false;

    QListWidgetItem *item;

    ~Pipeline()
    {
        delete item;
    }

	void setTitle()
	{
		QString title;
		{
			if (filename.compare(0, shaderPath.size(), shaderPath) == 0)
				title = filename.c_str() + shaderPath.size();
			else
				title = filename.c_str();
		}
		if (changed) title += "*";
		item->setText(title);
	}

    void addToTree()
    {
        item = new QListWidgetItem;
		setTitle();
		pipelineList->addItem(item);
    }

	Stage *stageByType(int type)
	{
		for (auto s : stages)
		{
			if ((int)s->type == type)
				return s;
		}
		return nullptr;
	}

	Stage *stageByTabIndex(int index)
	{
		for (auto &s : stages)
		{
			if (s->wrap.tabIndex == index)
				return s;
		}
		return nullptr;
	}

	void removeStage(Stage *p)
	{
		for (auto it = stages.begin(); it != stages.end(); it++)
		{
			if ((*it) == p)
			{
				auto s = *it;
				stages.erase(it);
				delete s;
				return;
			}
		}
	}

    void load(const std::string &_filename)
    {
		setFilename(_filename);
		loadXML();

		for (auto &s : stages)
		{
			tke::OnceFileBuffer file(filepath + "/" + s->filename);
			s->text = file.data;
		}

        addToTree();
    }

    void setTabData(int index)
    {
		auto s = stageByTabIndex(index);
		explorerButton->setStatusTip(s->filename.c_str());
		outputText->setPlainText(s->output.c_str());
		compileText->setText(s->compileOutput.c_str());
    }

    void appear()
    {
		attributeTree->clear();

        stageTab->clear();
		for (int type = 0; type < 5; type++)
		{
			auto s = stageByType(stageTypes[type]);
			if (s)
			{
				s->appear();
				s->wrap.tabIndex = stageTab->addTab(s->wrap.edit, "");
				s->wrap.setTitle();
			}
		}

		setTabData(0);
    }
	void on_save()
	{
		if (!changed) return;

		saveXML();

		changed = false;
		setTitle();
	}
};

std::vector<Pipeline*> pipelines;
Pipeline *currentPipeline = nullptr;

Stage *currentTabStage()
{
	if (!currentPipeline) return nullptr;
	return currentPipeline->stageByTabIndex(stageTab->currentIndex());
}

void Stage::on_save()
{
	if (!wrap.changed) return;

	text = wrap.edit->toPlainText().toUtf8().data();

	std::ofstream file(currentPipeline->filepath + "/" + filename);
	file.write(text.c_str(), text.size());

	wrap.changed = false;
	wrap.setTitle();
}

void Stage::on_spv()
{
	on_save();

	output = getFullText(currentPipeline->filepath);

	std::ofstream file("temp.glsl");
	file.write(output.c_str(), output.size());
	file.close();

	std::experimental::filesystem::path spv(currentPipeline->filepath + "/" + filename + ".spv");

	tke::exec("cmd", "/C glslangValidator my.conf -V temp.glsl -S " + stageNames[(int)log2((int)type)] + " -q -o " + spv.string() + " > output.txt");

	tke::OnceFileBuffer output("output.txt");
	{
		auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::stringstream ss;
		ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
		compileOutput = ss.str() + "\n" + "Warnning:push constants in different stages must be merged, or else they would not reflect properly.\n" + std::string(output.data);
	}

	{
		// analyzing the reflection

		enum ReflectionType
		{
			eNull = -1,
			eUniform = 0,
			eUniformBlock = 1,
			eVertexAttribute = 2
		};

		ReflectionType currentReflectionType = eNull;

		struct Reflection
		{
			int COUNT = 1; // special for UBO

			ReflectionType reflectionType;
			std::string name;
			int offset;
			std::string type;
			int size;
			int index;
			int binding;
		};
		struct ReflectionManager
		{
			std::vector<Reflection> rs;
			void add(Reflection &_r)
			{
				if (_r.reflectionType == eUniformBlock && _r.binding != -1)
				{
					for (auto &r : rs)
					{
						if (r.binding == _r.binding)
						{
							r.COUNT++;
							return;
						}
					}
				}
				rs.push_back(_r);
			}
		};
		ReflectionManager reflections;
		Reflection currentReflection;

		yylex_destroy();

		yyin = fopen("output.txt", "rb");
		int token = yylex();
		std::string last_string;
		while (token)
		{
			switch (token)
			{
			case COLON:
				if (currentReflectionType != eNull)
				{
					if (currentReflection.name != "") reflections.add(currentReflection);
					currentReflection.reflectionType = currentReflectionType;
					currentReflection.name = last_string;
					last_string = "";
				}
				break;
			case IDENTIFIER:
			{
				std::string string(yytext);
				if (string == "ERROR")
				{
					QMessageBox::information(0, "Oh Shit", "Shader Compile Failed", QMessageBox::Ok);
					token = 0;
				}
				last_string = string;
			}
			break;
			case VALUE:
			{
				std::string string(yytext);
				if (currentReflectionType != eNull)
				{
					if (last_string == "offset")
						currentReflection.offset = std::stoi(string);
					else if (last_string == "type")
						currentReflection.type = string;
					else if (last_string == "size")
						currentReflection.size = std::stoi(string);
					else if (last_string == "index")
						currentReflection.index = std::stoi(string);
					else if (last_string == "binding")
						currentReflection.binding = std::stoi(string);
				}
			}
			break;
			case UR_MK:
				currentReflectionType = eUniform;
				break;
			case UBR_MK:
				currentReflectionType = eUniformBlock;
				break;
			case VAR_MK:
				currentReflectionType = eVertexAttribute;
				break;
			}
			if (token) token = yylex();
		}
		if (currentReflection.name != "") reflections.add(currentReflection);
		fclose(yyin);
		yyin = NULL;

		descriptors.clear();
		pushConstantRanges.clear();
		for (auto &r : reflections.rs)
		{
			switch (r.reflectionType)
			{
			case eUniform:
				if (r.binding != -1 && r.type == "8b5e") // SAMPLER
				{
					tke::Descriptor d;
					d.type = tke::DescriptorType::sampler;
					d.name = r.name;
					d.binding = r.binding;
					d.count = r.size;
					descriptors.push_back(d);
				}
				break;
			case eUniformBlock:
				if (r.binding != -1) // UBO
				{
					tke::Descriptor d;
					d.type = tke::DescriptorType::uniform_buffer;
					d.name = r.name;
					d.binding = r.binding;
					d.count = r.COUNT;
					descriptors.push_back(d);
				}
				else // PC
				{
					tke::PushConstantRange p;
					p.offset = 0; // 0 always
					p.size = r.size;
					pushConstantRanges.push_back(p);
				}
				break;
			}
		}
	}

	DeleteFileA("output.txt");
	DeleteFileA("temp.glsl");
}
