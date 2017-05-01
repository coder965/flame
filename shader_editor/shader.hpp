struct Pipeline;

struct Stage : tke::StageAbstract
{
	std::string text;
	bool changed = false;
	std::string output;
	std::string compileOutput;

    std::string getFullText();
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

    void qTextToStageStr(int index)
    {
		for (auto &s : stages)
		{
			if ((int)s.type == index)
			{
				s.text = qTexts[index]->toPlainText().toUtf8().data();
				return;
			}
		}
    }

    void setTabData(int index)
    {
        for (int i = 0; i < 5; i++)
        {
            if (qTabIndexs[i] == index)
            {
				for (auto &s : stages)
				{
					if ((int)s.type == index)
					{
						explorerButton->setStatusTip(s.filename.c_str());
						outputMyEdit->setPlainText(s.output.c_str());
						compileTextBrowser->setText(s.compileOutput.c_str());
						return;
					}
				}
                return;
            }
        }
    }

    void refreshTabs()
    {
        stageTabWidget->clear();
        for (int i = 0; i < 5; i++)
        {
            qTabIndexs[i] = -1;
            auto stage = stages[i];
            if (stage)
            {
                if (stage->str) qTexts[i]->setPlainText(stage->str);
                QString title = stageNames[stage->type];
                if (stage->changed) title += "*";
                qTabIndexs[i] = stageTabWidget->addTab(qTexts[i], title);
            }
        }

        setTabData(0);
    }
};

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
			strOut += QString(file.data) + "\n";
		}
		else
		{
			strOut += str + "\n";
		}
	}
    return strOut;
}

std::vector<Pipeline*> pipelines;
Pipeline *currentPipeline = nullptr;
