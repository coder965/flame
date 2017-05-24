#include "world_editor.h"

#include "pipeline_editor_yy_def.h"
extern "C" {
	extern FILE *pipeline_editor_yyin;
	extern int pipeline_editor_yylex();
	int pipeline_editor_yylex_destroy();
	extern char *pipeline_editor_yytext;
}
namespace PipelineEditor
{
	struct PipelineExt : tke::ExtType
	{
		bool changed = false;
		QListWidgetItem *item = nullptr;
		PipelineExt()
		{
			item = new QListWidgetItem;
		}
		~PipelineExt()
		{
			delete item;
		}
	};

	struct StageExt : tke::ExtType
	{
		bool changed = false;
		std::string text;
		std::string outputText;
		std::string compileText;
	};

	std::string getStageFullText(const std::string &parent_path, tke::Stage *pStage)
	{
		auto ext = (StageExt*)pStage->ext;

		std::stringstream strIn(ext->text);
		std::string strOut;

		std::string line;
		while (!strIn.eof())
		{
			std::getline(strIn, line);

			std::regex pat(R"(#include\s+\"([\w\.\\]+)\")");
			std::smatch sm;
			if (std::regex_search(line, sm, pat))
			{
				auto include = sm[1].str();
				tke::OnceFileBuffer file(parent_path + "/" + pStage->filepath + "/" + include);
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

	std::vector<tke::Pipeline*> pipelines;
	tke::Pipeline *currentPipeline = nullptr;
	QLineNumberEdit *stageEdit[5];
	QFindWidget *findWidget;

	bool preparingData = false;

	void saveDataXml()
	{
		tke::AttributeTree at("data");

		for (auto pipeline : pipelines)
		{
			auto node = new tke::AttributeTreeNode("pipeline");
			node->attributes.emplace_back(new tke::NormalVariable("filename", &pipeline->filename), std::string());
		}

		at.saveXML("pipelines.xml");
	}
}

WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);

	{ // setup scene window
		while (!tke::currentWindow) Sleep(100);

		QWindow *window = QWindow::fromWinId((unsigned int)tke::currentWindow->hWnd);
		window->setFlags(Qt::FramelessWindowHint);

		QWidget *widget = QWidget::createWindowContainer(window);
		widget->setFixedWidth(1600);
		widget->setFixedHeight(900);

		ui.scrollArea->setWidget(widget);
	}

	{ // setup pipeline editor
		PipelineEditor::stageEdit[0] = ui.vertEdit;
		PipelineEditor::stageEdit[1] = ui.tescEdit;
		PipelineEditor::stageEdit[2] = ui.teseEdit;
		PipelineEditor::stageEdit[3] = ui.geomEdit;
		PipelineEditor::stageEdit[4] = ui.fragEdit;

		PipelineEditor::findWidget = new QFindWidget;
		ui.stageTab->setCornerWidget(PipelineEditor::findWidget->group);

		tke::AttributeTree at("data");
		at.loadXML("pipelines.xml");
		for (auto c : at.children)
		{
			if (c->name == "pipeline")
			{
				auto a = c->firstAttribute("filename");
				auto pipeline = new tke::Pipeline;
				pipeline->setFilename(a.second);
				pipeline->loadXML();
				pipeline->ext = new PipelineEditor::PipelineExt;
				for (int i = 0; i < 5; i++)
				{
					auto s = pipeline->stages[i];
					if (s)
					{
						auto ext = new PipelineEditor::StageExt;
						s->ext = ext; 
						tke::OnceFileBuffer file(pipeline->filepath + "/" + s->filename);
						ext->text = file.data;
					}
				}
				PipelineEditor::pipelines.push_back(pipeline);

				((PipelineEditor::PipelineExt*)pipeline->ext)->item->setText(a.second.c_str());
				ui.pipelineList->addItem(((PipelineEditor::PipelineExt*)pipeline->ext)->item);
			}
		}
	}

	{ // setup renderer editor

	}
}

void WorldEditor::keyPressEvent(QKeyEvent *k)
{
	if (k->key() == Qt::Key_S)
	{
		if (k->modifiers() == Qt::ControlModifier)
			/*on_saveStage_clicked()*/;
	}
	else if (k->key() == Qt::Key_F5)
	{
		/*on_toSpv_clicked()*/;
	}
}

void WorldEditor::on_pipelineList_currentItemChanged(QListWidgetItem *curr, QListWidgetItem *)
{
	for (auto pipeline : PipelineEditor::pipelines)
	{
		if (((PipelineEditor::PipelineExt*)pipeline->ext)->item == curr)
		{
			if (pipeline == PipelineEditor::currentPipeline) return;

			if (PipelineEditor::currentPipeline)
			{
				for (int i = 0; i < 5; i++)
				{
					auto s = PipelineEditor::currentPipeline->stages[i];
					if (s)
						((PipelineEditor::StageExt*)s->ext)->text = PipelineEditor::stageEdit[i]->toPlainText().toUtf8().data();
				}
			}

			PipelineEditor::currentPipeline = pipeline;

			PipelineEditor::preparingData = true;
			for (int i = 0; i < 5; i++)
			{
				auto s = PipelineEditor::currentPipeline->stages[i];
				if (s)
				{
					auto ext = (PipelineEditor::StageExt*)s->ext;
					if (!ext->changed)
						ui.stageTab->setTabText(i, tke::StageNames[i].c_str());
					else
						ui.stageTab->setTabText(i, (tke::StageNames[i] + "*").c_str());
					PipelineEditor::stageEdit[i]->setPlainText(ext->text.c_str());
					PipelineEditor::stageEdit[i]->setEnabled(true);
				}
				else
				{
					ui.stageTab->setTabText(i, tke::StageNames[i].c_str());
					PipelineEditor::stageEdit[i]->setPlainText("");
					PipelineEditor::stageEdit[i]->setEnabled(false);
				}
			}
			on_stageTab_currentChanged(ui.stageTab->currentIndex());
			PipelineEditor::preparingData = false;


			return;
		}
	}
}

void WorldEditor::stage_edit_changed(int i)
{
	if (PipelineEditor::preparingData) return; 
	((PipelineEditor::StageExt*)PipelineEditor::currentPipeline->stages[i]->ext)->changed = true;
	ui.stageTab->setTabText(i, (tke::StageNames[i] + "*").c_str());
}

void WorldEditor::on_vertEdit_textChanged()
{
	stage_edit_changed(0);
}

void WorldEditor::on_tescEdit_textChanged() 
{
	stage_edit_changed(1);
}

void WorldEditor::on_teseEdit_textChanged() 
{
	stage_edit_changed(2);
}

void WorldEditor::on_geomEdit_textChanged()
{
	stage_edit_changed(3);
}

void WorldEditor::on_fragEdit_textChanged()
{
	stage_edit_changed(4);
}

void WorldEditor::on_addPipeline_clicked()
{
	auto list = QFileDialog::getOpenFileNames(this);
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto pipeline = new tke::Pipeline;
		pipeline->setFilename(list[i].toUtf8().data());
		pipeline->loadXML();
		pipeline->ext = new PipelineEditor::PipelineExt;
		PipelineEditor::pipelines.push_back(pipeline);

		((PipelineEditor::PipelineExt*)pipeline->ext)->item->setText(pipeline->filename.c_str());
		ui.pipelineList->addItem(((PipelineEditor::PipelineExt*)pipeline->ext)->item);
	}

	PipelineEditor::saveDataXml();
}

void WorldEditor::on_removePipeline_clicked()
{
	if (!PipelineEditor::currentPipeline) return;

	for (auto it = PipelineEditor::pipelines.begin(); it != PipelineEditor::pipelines.end(); it++)
	{
		if (*it == PipelineEditor::currentPipeline)
		{
			PipelineEditor::pipelines.erase(it);
			break;
		}
	}

	delete PipelineEditor::currentPipeline;
	PipelineEditor::currentPipeline = nullptr;

	PipelineEditor::saveDataXml();
}

void WorldEditor::save_pipeline(tke::Pipeline *p)
{
	auto ext = (PipelineEditor::PipelineExt*)p->ext;
	if (!ext->changed) return;
	p->saveXML();
	ext->item->setText(p->filename.c_str());
	ext->changed = false;
}

void WorldEditor::save_stage(int i)
{
	auto s = PipelineEditor::currentPipeline->stages[i];
	if (!s) return;

	auto ext = (PipelineEditor::StageExt*)s->ext;

	if (!ext->changed) return;

	ext->text = PipelineEditor::stageEdit[i]->toPlainText().toUtf8().data();

	std::ofstream file(PipelineEditor::currentPipeline->filepath + "/" + s->filename);
	file.write(ext->text.c_str(), ext->text.size());

	ext->changed = false;
	ui.stageTab->setTabText(i, tke::StageNames[i].c_str());
}

void WorldEditor::stage_to_spv(int i)
{
	auto s = PipelineEditor::currentPipeline->stages[i];
	if (!s) return;

	auto ext = (PipelineEditor::StageExt*)s->ext;

	save_stage(i);

	ext->outputText = PipelineEditor::getStageFullText(PipelineEditor::currentPipeline->filepath, s);

	std::ofstream file("temp.glsl");
	file.write(ext->outputText.c_str(), ext->outputText.size());
	file.close();

	std::experimental::filesystem::path spv(PipelineEditor::currentPipeline->filepath + "/" + s->filename + ".spv");

	tke::exec("cmd", "/C glslangValidator my_glslValidator_config.conf -V temp.glsl -S " + tke::StageNames[i] + " -q -o " + spv.string() + " > output.txt");

	tke::OnceFileBuffer output("output.txt");
	{
		auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::stringstream ss;
		ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
		ext->compileText = ss.str() + "\n" + "Warnning:push constants in different stages must be merged, or else they would not reflect properly.\n" + std::string(output.data);
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

		pipeline_editor_yylex_destroy();

		pipeline_editor_yyin = fopen("output.txt", "rb");
		int token = pipeline_editor_yylex();
		std::string last_string;
		while (token)
		{
			switch (token)
			{
			case PIPELINE_EDITOR_YY_COLON:
				if (currentReflectionType != eNull)
				{
					if (currentReflection.name != "") reflections.add(currentReflection);
					currentReflection.reflectionType = currentReflectionType;
					currentReflection.name = last_string;
					last_string = "";
				}
				break;
			case PIPELINE_EDITOR_YY_IDENTIFIER:
			{
				std::string string(pipeline_editor_yytext);
				if (string == "ERROR")
				{
					QMessageBox::information(0, "Oh Shit", "Shader Compile Failed", QMessageBox::Ok);
					token = 0;
				}
				last_string = string;
			}
				break;
			case PIPELINE_EDITOR_YY_VALUE:
			{
				std::string string(pipeline_editor_yytext);
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
			case PIPELINE_EDITOR_YY_UR_MK:
				currentReflectionType = eUniform;
				break;
			case PIPELINE_EDITOR_YY_UBR_MK:
				currentReflectionType = eUniformBlock;
				break;
			case PIPELINE_EDITOR_YY_VAR_MK:
				currentReflectionType = eVertexAttribute;
				break;
			}
			if (token) token = pipeline_editor_yylex();
		}
		if (currentReflection.name != "") reflections.add(currentReflection);
		fclose(pipeline_editor_yyin);
		pipeline_editor_yyin = NULL;

		s->descriptors.clear();
		s->pushConstantRanges.clear();
		for (auto &r : reflections.rs)
		{
			switch (r.reflectionType)
			{
			case eUniform:
				if (r.binding != -1 && r.type == "8b5e") // SAMPLER
				{
					tke::Descriptor d;
					d.type = tke::DescriptorType::image_n_sampler;
					d.name = r.name;
					d.binding = r.binding;
					d.count = r.size;
					s->descriptors.push_back(d);
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
					s->descriptors.push_back(d);
				}
				else // PC
				{
					tke::PushConstantRange p;
					p.offset = 0; // 0 always
					p.size = r.size;
					s->pushConstantRanges.push_back(p);
				}
				break;
			}
		}
	}

	DeleteFileA("output.txt");
	DeleteFileA("temp.glsl");
}

void WorldEditor::on_savePipeline_clicked()
{
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		for (auto p : PipelineEditor::pipelines)
			save_pipeline(p);
	}
	else
	{
		if (!PipelineEditor::currentPipeline) return;
		save_pipeline(PipelineEditor::currentPipeline);
	}
}

void WorldEditor::on_explorerPipeline_clicked()
{
	if (!PipelineEditor::currentPipeline) return;

	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		ShellExecuteA(NULL, "open", PipelineEditor::currentPipeline->filename.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::experimental::filesystem::path p(PipelineEditor::currentPipeline->filename);
		std::string cmd("explorer /select," + std::experimental::filesystem::absolute(p).string());
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}

void WorldEditor::on_stageTab_currentChanged(int i)
{
	auto s = PipelineEditor::currentPipeline->stages[i];
	if (s)
	{
		auto ext = (PipelineEditor::StageExt*)s->ext;
		ui.explorerStage->setStatusTip(s->filename.c_str());
		ui.outputText->setPlainText(ext->outputText.c_str());
		ui.compileText->setText(ext->compileText.c_str());
		PipelineEditor::findWidget->on_find();
	}
	else
	{
		ui.explorerStage->setStatusTip("");
		ui.outputText->setPlainText("");
		ui.compileText->setText("");
	}
}

void WorldEditor::on_addStage_clicked()
{
	if (!PipelineEditor::currentPipeline || 
		(PipelineEditor::currentPipeline->stages[0] &&
		 PipelineEditor::currentPipeline->stages[1] &&
		 PipelineEditor::currentPipeline->stages[2] &&
		 PipelineEditor::currentPipeline->stages[3] &&
		 PipelineEditor::currentPipeline->stages[4])
		) return;

	bool ok;
	QString text = QInputDialog::getText(this, "File Name",
		"(Create when not exist)",
		QLineEdit::Normal, "", &ok);

	if (!ok) return;

	std::string filename = text.toUtf8().data();
	std::experimental::filesystem::path p(filename);
	auto type = tke::StageFlagByExt(p.extension().string());

	for (int i = 0; i < 5; i++)
	{
		if (tke::StageTypes[i] == (int)type)
		{
			if (PipelineEditor::currentPipeline->stages[i])
				return;

			auto s = new tke::Stage;
			s->type = type;
			s->filename = filename;
			s->filepath = p.parent_path().string();

			auto ext = new PipelineEditor::StageExt;
			s->ext = ext;
			tke::OnceFileBuffer file(PipelineEditor::currentPipeline->filepath + "/" + s->filename);
			ext->text = file.data;

			PipelineEditor::currentPipeline->stages[i] = s;

			PipelineEditor::preparingData = true;
			ui.stageTab->setTabText(i, tke::StageNames[i].c_str());
			PipelineEditor::stageEdit[i]->setPlainText(ext->text.c_str());
			PipelineEditor::stageEdit[i]->setEnabled(true);
			PipelineEditor::preparingData = false;

			((PipelineEditor::PipelineExt*)PipelineEditor::currentPipeline->ext)->changed = true;
			((PipelineEditor::PipelineExt*)PipelineEditor::currentPipeline->ext)->item->setText((PipelineEditor::currentPipeline->filename + "*").c_str());
		}
	}
}

void WorldEditor::on_removeStage_clicked()
{
	if (!PipelineEditor::currentPipeline) return;
	auto i = ui.stageTab->currentIndex();
	if (!PipelineEditor::currentPipeline->stages[i]) return;

	delete PipelineEditor::currentPipeline->stages[i];
	PipelineEditor::currentPipeline->stages[i] = nullptr;

	ui.stageTab->setTabText(i, tke::StageNames[i].c_str());
	PipelineEditor::stageEdit[i]->setPlainText("");
	PipelineEditor::stageEdit[i]->setEnabled(false);
	ui.explorerStage->setStatusTip("");
	ui.outputText->setPlainText("");
	ui.compileText->setText("");

	((PipelineEditor::PipelineExt*)PipelineEditor::currentPipeline->ext)->changed = true;
	((PipelineEditor::PipelineExt*)PipelineEditor::currentPipeline->ext)->item->setText((PipelineEditor::currentPipeline->filename + "*").c_str());
}

void WorldEditor::on_saveStage_clicked()
{
	if (!PipelineEditor::currentPipeline) return;

	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		for (int i = 0; i < 5; i++)
			save_stage(i);
	}
	else
	{
		save_stage(ui.stageTab->currentIndex());
	}
}

void WorldEditor::on_toSpv_clicked()
{
	if (!PipelineEditor::currentPipeline) return;

	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		for (int i = 0; i < 5; i++)
			stage_to_spv(i);
	}
	else
	{
		stage_to_spv(ui.stageTab->currentIndex());
	}

	on_stageTab_currentChanged(ui.stageTab->currentIndex());

	((PipelineEditor::PipelineExt*)PipelineEditor::currentPipeline->ext)->changed = true;
	((PipelineEditor::PipelineExt*)PipelineEditor::currentPipeline->ext)->item->setText((PipelineEditor::currentPipeline->filename + "*").c_str());
}


void WorldEditor::on_explorerStage_clicked()
{
	auto s = PipelineEditor::currentPipeline->stages[ui.stageTab->currentIndex()];
	if (!s) return;

	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		std::string cmd(PipelineEditor::currentPipeline->filepath + "/" + s->filename);
		ShellExecuteA(NULL, "open", cmd.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::experimental::filesystem::path p(PipelineEditor::currentPipeline->filepath + "/" + s->filename);
		std::string cmd("explorer /select," + std::experimental::filesystem::absolute(p).string());
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}
