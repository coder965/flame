struct Pipeline;

struct Stage
{
    Pipeline *pipeline;

    std::string filename;
    std::string filepath;

    int type;
	std::string str;

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

int getStageIndex(const std::string &str)
{
    if (str == ".vert") return 0;
    else if (str == ".tesc") return 1;
    else if (str == ".tese") return 2;
    else if (str == ".geom") return 3;
    else if (str == ".frag") return 4;

    return -1;
}

struct Pipeline
{
    std::string name;

    std::string filename;
    std::string filepath;

    Stage *stages[5];

    QTreeWidgetItem *item;

	tk::StrPool strPool;
    rapidxml::xml_document<> xmlDoc;
    rapidxml::xml_node<> *xmlRootNode;
    rapidxml::xml_node<> *xmlShaderNode;

    Pipeline()
    {
        name[0] = 0;

        for (int i = 0; i < 5; i++)
            stages[i] = nullptr;

		strPool.init(2048);
        xmlRootNode = xmlDoc.allocate_node(rapidxml::node_element, "pipeline");
        xmlShaderNode = xmlDoc.allocate_node(rapidxml::node_element, "shader");
        xmlRootNode->append_node(xmlShaderNode);
        xmlDoc.append_node(xmlRootNode);
    }

    ~Pipeline()
    {
        for (int i = 0; i < 5; i++) delete stages[i];
        delete item;
		strPool.destroy();
        xmlDoc.clear();
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

    void load(const char *_filename)
    {
        strcpy(filename, _filename);
        tk::getFilePath(filename, filepath);

        char *file;
        tk::loadFile(filename, &file);

        if (file)
        {
			strPool.reset();

            rapidxml::xml_document<> loadedXmlDoc;
			loadedXmlDoc.parse<0>(file);
            auto firstNode = loadedXmlDoc.first_node();
            if (strcmp(firstNode->name(), "pipeline") == 0)
            {
                for (auto a = firstNode->first_attribute(); a; a = a->next_attribute())
                {
                    auto attribute = xmlDoc.allocate_attribute(strPool.add(a->name()), strPool.add(a->value()));
                    xmlRootNode->append_attribute(attribute);
                }

                for (auto n = firstNode->first_node(); n; n = n->next_sibling())
                {
                    if (strcmp(n->name(), "shader") == 0)
                    {
						auto shaderNode = n;
                        for (auto nn = shaderNode->first_node(); nn; nn = nn->next_sibling())
                        {
                            if (strcmp(nn->name(), "stage") == 0)
                            {
                                auto stage = new Stage;
                                stage->pipeline = this;
                                for (auto a = nn->first_attribute(); a; a = a->next_attribute())
                                {
                                    if (strcmp(a->name(), "filename") == 0)
                                    {
                                        strcpy(stage->filename, a->value());
                                        tk::getFilePath(stage->filename, stage->filepath);

                                        char ext[32];
                                        tk::getFileExt(stage->filename, ext);
                                        stage->type = getStageIndex(ext);

                                        char filename[260];
                                        sprintf(filename, "%s%s", filepath, stage->filename);
                                        tk::loadFile(filename, &stage->str);
                                        stage->changed = false;
                                    }
                                }
                                stages[stage->type] = stage;
                            }
                        }
                    }
                    else
                    {
                        reserveXml(n, xmlRootNode);
                    }
                }
            }
			loadedXmlDoc.clear();
            delete[] file;
        }

        addToTree();
    }

    bool isAllFull()
    {
        for (int i = 0; i < 5; i++)
        {
            if (!stages[i])
                return false;
        }
        return true;
    }

    void reserveXml(rapidxml::xml_node<> *srcNode, rapidxml::xml_node<> *dstParent)
    {
        auto node = xmlDoc.allocate_node(rapidxml::node_element, strPool.add(srcNode->name()));
        for (auto a = srcNode->first_attribute(); a; a = a->next_attribute())
        {
            auto attribute = xmlDoc.allocate_attribute(strPool.add(a->name()), strPool.add(a->value()));
            node->append_attribute(attribute);
        }
        dstParent->append_node(node);

        for (auto n = srcNode->first_node(); n; n = n->next_sibling())
        {
            reserveXml(n, node);
        }
    }

    void saveXml()
    {
		std::vector<rapidxml::xml_node<> *> nodes;
		std::vector<rapidxml::xml_attribute<> *> attributes;

        for (int i = 0; i < 5; i++)
        {
            auto stage = stages[i];
            if (stage)
            {
                auto stageNode = new rapidxml::xml_node<>(rapidxml::node_element);
				nodes.push_back(stageNode);
				stageNode->name("stage");

                auto stageAttribute = new rapidxml::xml_attribute<>();
				attributes.push_back(stageAttribute);
				stageAttribute->name("filename");
				stageAttribute->value(stage->filename);
				stageNode->append_attribute(stageAttribute);

                xmlShaderNode->append_node(stageNode);
            }
        }

        std::string dst;
        rapidxml::print(std::back_inserter(dst), xmlDoc);

        auto f = fopen(filename, "wb");
        fprintf(f, dst.c_str());
        fclose(f);

        xmlShaderNode->remove_all_nodes();

        for (auto node : nodes) delete node;
		for (auto attribute : attributes) delete attribute;
    }

    void qTextToStageStr(int index)
    {
        tk::setStr(&stages[index]->str, qTexts[index]->toPlainText().toUtf8().data());
    }

    void setTabData(int index)
    {
        for (int i = 0; i < 5; i++)
        {
            if (qTabIndexs[i] == index)
            {
                auto stage = stages[i];
                explorerButton->setStatusTip(stage->filename);
                outputMyEdit->setPlainText(stage->output);
                compileTextBrowser->setText(stage->compileOutput);
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

QString Stage::getFullText()
{
    QString strIn(str);
    QString strOut;
    strOut += "#version 450 core\n";
    strOut += "#extension GL_ARB_separate_shader_objects : enable\n";
    strOut += "#extension GL_ARB_shading_language_420pack : enable\n";
    strOut += "\n";

    auto list = strIn.split("\n");
    for (int i = 0; i < list.size(); i++)
    {
        auto str = list[i];

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
