struct Pipeline;

const std::string stageNames[] = {
	"vert",
	"tesc",
	"tese",
	"geom",
	"frag"
};

struct StageWrap : QObject
{
	int type;

	bool changed = false;
	MyEdit *edit = nullptr;
	int tabIndex = -1;
	~StageWrap(){ delete edit; }
	void appear()
	{
		edit = new MyEdit;
		edit->setLineWrapMode(QPlainTextEdit::NoWrap);
		connect(edit, &QPlainTextEdit::textChanged, this, &StageWrap::on_text_changed);
	}

	void on_text_changed()
	{
		if (qTextDataPreparing) return;

		changed = true;
		stageTabWidget->setTabText(tabIndex, QString(stageNames[type].c_str()) + "*");
	}
};

struct Stage : tke::StageAbstract
{
	std::string text;
	std::string output;
	std::string compileOutput;

	StageWrap wrap;

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
				tke::OnceFileBuffer file(parent_path + "/" + filepath + include);
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
};

struct Pipeline : tke::PipelineAbstract<Stage>
{
    QTreeWidgetItem *item;

    ~Pipeline()
    {
        delete item;
    }

    void addToTree()
    {
        QString _filename;
        {
            auto length = strlen(shaderPath);
            if (filename.compare(0, length, shaderPath) == 0)
                _filename = filename.c_str() + length;
            else
                _filename = filename.c_str();
        }

        item = new QTreeWidgetItem;
		item->setText(0, _filename);
		pipelineTree->addTopLevelItem(item);
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
		outputMyEdit->setPlainText(s->output.c_str());
		compileTextBrowser->setText(s->compileOutput.c_str());
    }

    void refreshTabs()
    {
        stageTabWidget->clear();
		int i = 0;
		for (int type = 0; type < 5; type++)
		{
			auto s = stageByType(type);
			if (s)
			{
				s->appear();
				QString title = stageNames[(int)type].c_str();
				if (s->wrap.changed) title += "*";
				s->wrap.tabIndex = stageTabWidget->addTab(s->wrap.edit, title);
			}
		}

        setTabData(0);
    }
};

std::vector<Pipeline*> pipelines;
Pipeline *currentPipeline = nullptr;

Stage *currentTabStage()
{
	if (!currentPipeline) return nullptr;
	return currentPipeline->stageByTabIndex(stageTabWidget->currentIndex());
}