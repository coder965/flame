struct Pipeline;

struct Stage : tke::StageAbstract
{
	std::string text;
	bool changed = false;
	std::string output;
	std::string compileOutput;

	MyEdit *edit = nullptr;
	int tabIndex = -1;

	void appear()
	{
		edit = new MyEdit;
		edit->setLineWrapMode(QPlainTextEdit::NoWrap);
	}

	void disappear() 
	{
		delete edit;
		tabIndex = -1;
	}

	std::string Stage::getFullText()
	{
		std::stringstream strIn(text);
		std::string strOut;
		strOut += "#version 450 core\n";
		strOut += "#extension GL_ARB_separate_shader_objects : enable\n";
		strOut += "#extension GL_ARB_shading_language_420pack : enable\n";
		strOut += "\n";

		std::string line;
		while (strIn.eof())
		{
			std::getline(strIn, line);
			QString pat = R"(#include\s+\"([\w\.\\]+)\")";
			QRegExp reg(pat);
			auto firstPos = reg.indexIn(str);
			if (firstPos >= 0)
			{
				auto include = reg.cap(1);
				char _filename[260];
				sprintf(_filename, "%s%s%s", pipeline->filepath, filepath, include.toUtf8().data());

				tke::OnceFileBuffer file(_filename);
				strOut += file.data;
				strOut += "\n";
			}
			else
			{
				strOut += str + "\n";
			}
		}
		return strOut;
	}

	void on_text_changed();
};

const char* stageNames[] = {
    "vert",
    "tesc",
    "tese",
    "geom",
    "frag"
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

	Stage *getStage(int type)
	{
		for (auto &s : stages)
		{
			if ((int)s.type == type)
				return &s;
		}
	}

    void load(const std::string &_filename)
    {
		setFilename(_filename);
		loadXML();

		for (auto &s : stages)
		{
			tke::OnceFileBuffer file(filepath + "/" + s.filename);
			s.text = file.data;
		}

        addToTree();
    }

    void saveXml()
    {
		saveXML();
    }

    void qTextToStageText(int type)
    {
		getStage(type)->text = qTexts[type]->toPlainText().toUtf8().data();
    }

    void setTabData(int index)
    {
		auto s = getStage(qTabTypes[index]);
		explorerButton->setStatusTip(s->filename.c_str());
		outputMyEdit->setPlainText(s->output.c_str());
		compileTextBrowser->setText(s->compileOutput.c_str());
    }

    void refreshTabs()
    {
        stageTabWidget->clear();
		int i = 0;
		for (auto &s : stages)
		{
			qTexts[i]->setPlainText(s.text.c_str());
			QString title = stageNames[(int)s.type];
			if (stage->changed) title += "*";
			qTabIndexs[i] = stageTabWidget->addTab(qTexts[i], title);
			qTabTypes[i] = (int)s.type;
			i++;
		}

        setTabData(0);
    }
};

std::vector<Pipeline*> pipelines;
Pipeline *currentPipeline = nullptr;

Stage *getCurrentStage()
{
	if (!currentPipeline) return nullptr;
	auto index = stageTabWidget->currentIndex();
	if (index == -1) return nullptr;
	for (auto &s : currentPipeline->stages)
	{
		if (s.tabIndex == index)
			return &s;
	}
	return nullptr;
}