#include "world_editor.h"

static WorldEditor *worldEditor = nullptr;

static GameExplorer *gameExplorer = nullptr;
static StageEditor *currentStageEditor = nullptr;
static Monitor *currentMonitor = nullptr;

struct UserData : QObjectUserData
{
	int v;
	UserData(int _v)
	{
		v = _v;
	}
};
UserData *gameExplorerData = new UserData(WindowTypeGameExplorer);
UserData *stageEditorData = new UserData(WindowTypeStageEditor);
UserData *MonitorData = new UserData(WindowTypeMonitor);

static WindowType getCurrentWindowType()
{
	QWidget* wid = QApplication::focusWidget();
	QDockWidget* dock = 0;
	while (dock != (QDockWidget*)worldEditor && wid != 0)
	{
		dock = qobject_cast<QDockWidget*>(wid);
		if (dock)
			break;
		wid = (QWidget*)wid->parent();
	}
	if (dock)
	{
		auto data = (UserData*)dock->userData(0);
		switch (data->v)
		{
		case WindowTypeStageEditor:
			currentStageEditor = (StageEditor*)dock;
			break;
		}
		return (WindowType)data->v;
	}
}

void PipelineExt::setItemText()
{
	if (item)
		item->setText(0, QString(p->filename.c_str()) + (changed ? "*" : ""));
}

void PipelineExt::setChanged(bool _changed)
{
	changed = _changed;
	setItemText();
}

PipelineExt::PipelineExt(tke::Pipeline *_p)
{
	p = _p;
}

PipelineExt::~PipelineExt()
{
	delete item;
}

void StageExt::setItemText()
{
	if (item)
		item->setText(0, QString(tke::StageNameByType(p->type).c_str()) + " - \"" + p->filename.c_str() + "\"" + (changed ? "*" : ""));
}

void StageExt::setChanged(bool _changed)
{
	changed = _changed;
	setItemText();
}

StageExt::StageExt(tke::Stage *_p)
{
	p = _p;
}

StageExt::~StageExt()
{
	delete item;
	delete editor;
}

void Game::load()
{
	{ // load pipeles
		tke::AttributeTree at("data");
		at.loadXML("pipelines.xml");
		for (auto c : at.children)
		{
			if (c->name == "pipeline")
			{
				auto a = c->firstAttribute("filename");
				auto p = new tke::Pipeline;
				p->loadXML(a.second);
				p->ext = new PipelineExt(p);
				for (int i = 0; i < 5; i++)
				{
					auto s = p->stages[i];
					if (s)
					{
						auto ext = new StageExt(s);
						s->ext = ext;
						tke::OnceFileBuffer file(p->filepath + "/" + s->filename);
						ext->text = file.data;
					}
				}
				pipelines.push_back(p);
			}
		}
	}

	{ // load renderers
		tke::AttributeTree at("data");
		at.loadXML("renderers.xml");
		for (auto c : at.children)
		{
			if (c->name == "renderer")
			{
				auto a = c->firstAttribute("filename");
				auto r = new tke::Renderer;
				r->loadXML(a.second);
				renderers.push_back(r);
			}
		}
	}
}

void Game::save()
{
	{ // save pipelines
		tke::AttributeTree at("data");

		for (auto pipeline : pipelines)
		{
			auto node = new tke::AttributeTreeNode("pipeline");
			node->attributes.emplace_back(new tke::NormalVariable("filename", &pipeline->filename), std::string());
			at.children.push_back(node);
		}

		at.saveXML("pipelines.xml");
	}
	{ // TODO : save renderers

	}
}

static Game game;

void GameExplorer::on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev)
{
	for (auto p : game.pipelines)
	{
		auto ext = (PipelineExt*)p->ext;
		if (ext->item == curr)
		{
			currentItemType = ItemTypePipeline;
			currentPipeline = p;
			return;
		}
		for (int i = 0; i < 5; i++)
		{
			auto s = p->stages[i];
			if (s)
			{
				auto ext = (StageExt*)s->ext;
				if (ext->item == curr)
				{
					currentItemType = ItemTypeStage;
					currentStage = s;
					return;
				}
			}
		}
	}
	currentItemType = ItemTypeNull;
}

void GameExplorer::on_item_dbClicked(QTreeWidgetItem *item, int column)
{
	switch (currentItemType)
	{
	case ItemTypeStage:
	{
		auto ext = (StageExt*)currentStage->ext;
		if (!ext->editor)
		{
			ext->editor = new StageEditor(worldEditor);
			ext->editor->stage = currentStage;
			ext->editor->setup();
			worldEditor->addDockWidget(Qt::TopDockWidgetArea, ext->editor);
		}
	}
		break;
	}
}

void GameExplorer::setup()
{
	setUserData(0, gameExplorerData);

	setWindowTitle("Game Explorer");

	tree = new QTreeWidget;
	tree->setHeaderHidden(true);
	pipelinesItem = new QTreeWidgetItem;
	pipelinesItem->setText(0, "Pipelines");
	for (auto p : game.pipelines)
	{
		auto ext = (PipelineExt*)p->ext;
		ext->item = new QTreeWidgetItem;
		ext->setItemText();
		pipelinesItem->addChild(ext->item);
		for (int i = 0; i < 5; i++)
		{
			auto s = p->stages[i];
			if (s)
			{
				auto _ext = (StageExt*)s->ext;
				_ext->item = new QTreeWidgetItem;
				_ext->setItemText();
				ext->item->addChild(_ext->item);
			}
		}
	}
	tree->addTopLevelItem(pipelinesItem);
	renderersItem = new QTreeWidgetItem;
	renderersItem->setText(0, "Renderers");
	tree->addTopLevelItem(renderersItem);
	scenesItem = new QTreeWidgetItem;
	scenesItem->setText(0, "Scenes");
	tree->addTopLevelItem(scenesItem);
	setWidget(tree);
	connect(tree, &QTreeWidget::currentItemChanged, this, &GameExplorer::on_item_changed);
	connect(tree, &QTreeWidget::itemDoubleClicked, this, &GameExplorer::on_item_dbClicked);
}

void GameExplorer::closeEvent(QCloseEvent *event)
{
	for (auto p : game.pipelines)
	{
		auto ext = (PipelineExt*)p->ext;
		ext->item = nullptr;
		for (auto s : p->stages)
		{
			auto _ext = (StageExt*)s->ext;
			_ext->item = nullptr;
		}
	}
	delete gameExplorer;
	gameExplorer = nullptr;
}

void StageEditor::edit_changed()
{
	auto ext = (StageExt*)stage->ext;
	ext->changed = true;
	ext->item->setText(0, QString(tke::StageNameByType(stage->type).c_str()) + " - \"" + stage->filename.c_str() + "\"*");
}

void StageEditor::setup()
{
	setUserData(0, stageEditorData);

	auto _ext = (StageExt*)stage->ext;

	setWindowTitle(QString("Stage Editor - ") + stage->parent->name.c_str() + ":" + tke::StageNameByType(stage->type).c_str());

	edit = new QLineNumberEdit;
	edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
	edit->setPlainText(_ext->text.c_str());
	connect(edit, &QPlainTextEdit::textChanged, this, &StageEditor::edit_changed);
	//ext->editor->findWidget = new QFindWidget;
	//ext->editor->stageTab->setCornerWidget(ext->editor->findWidget->group);

	auto bottomTab = new QTabWidget;
	outputText = new QLineNumberEdit;
	outputText->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
	outputText->setPlainText(_ext->outputText.c_str());
	bottomTab->addTab(outputText, "Output");
	compileText = new QTextBrowser;
	compileText->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
	compileText->setPlainText(_ext->compileText.c_str());
	bottomTab->addTab(compileText, "Compile");

	auto splitter = new QSplitter;
	splitter->setOrientation(Qt::Orientation::Vertical);
	splitter->addWidget(edit);
	splitter->addWidget(bottomTab);

	setWidget(splitter);
}

void StageEditor::closeEvent(QCloseEvent *event)
{
	auto ext = (StageExt*)stage->ext;
	delete ext->editor;
	ext->editor = nullptr;
}

#include "pipeline_editor_yy_def.h"
extern "C" {
	extern FILE *pipeline_editor_yyin;
	extern int pipeline_editor_yylex();
	int pipeline_editor_yylex_destroy();
	extern char *pipeline_editor_yytext;
}

WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	worldEditor = this;

	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);

	game.load();

	{ // setup menus
		connect(ui.action_new_stage, &QAction::triggered, this, &WorldEditor::on_new_stage);
		connect(ui.action_new_pipeline, &QAction::triggered, this, &WorldEditor::on_new_pipeline);
		connect(ui.action_save_selected_item, &QAction::triggered, this, &WorldEditor::on_save_selected_item);
		connect(ui.action_save_all, &QAction::triggered, this, &WorldEditor::on_save_all);
		connect(ui.action_open_in_file_explorer, &QAction::triggered, this, &WorldEditor::on_open_in_file_explorer);
		connect(ui.action_remove, &QAction::triggered, this, &WorldEditor::on_remove);
		connect(ui.action_view_Game_Explorer, &QAction::triggered, this, &WorldEditor::on_view_game_explorer);
		connect(ui.action_compile, &QAction::triggered, this, &WorldEditor::on_compile);
	}

	{ // setup scene window
		//while (!tke::currentWindow) Sleep(100);

		//QWindow *window = QWindow::fromWinId((unsigned int)tke::currentWindow->hWnd);
		//window->setFlags(Qt::FramelessWindowHint);

		//QWidget *widget = QWidget::createWindowContainer(window);
		//widget->setFixedWidth(1600);
		//widget->setFixedHeight(900);

		//ui.scrollArea->setWidget(widget);
	}
}

void WorldEditor::keyPressEvent(QKeyEvent *k)
{
	if (k->key() == Qt::Key_S)
	{
		if (k->modifiers() == Qt::ControlModifier)
			on_save_selected_item();
		else if (k->modifiers() == Qt::ControlModifier | Qt::ShiftModifier)
			on_save_all();
	}
	else if (k->key() == Qt::Key_F5)
	{
		on_compile();
	}
}

void WorldEditor::on_new_stage()
{
	auto currentWindowType = getCurrentWindowType();
	switch (currentWindowType)
	{
	case WindowTypeGameExplorer:
		switch (gameExplorer->currentItemType)
		{
		case GameExplorer::ItemTypePipeline:
			if (gameExplorer->currentPipeline->isFullOfStage()) 
				return;

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
					if (gameExplorer->currentPipeline->stages[i])
						return;

					auto s = new tke::Stage(gameExplorer->currentPipeline);
					s->type = type;
					s->filename = filename;
					s->filepath = p.parent_path().string();

					filename = gameExplorer->currentPipeline->filepath + "/" + s->filename;
					if (!std::experimental::filesystem::exists(filename))
					{
						std::ofstream file(filename);
						file.close();
					}

					auto _ext = new StageExt(s);
					s->ext = _ext;
					tke::OnceFileBuffer file(filename);
					_ext->text = file.data;

					auto ext = (PipelineExt*)gameExplorer->currentPipeline->ext;
					{
						for (int i = 0; i < 5; i++)
						{
							auto s = gameExplorer->currentPipeline->stages[i];
							if (s)
							{
								auto ext = (StageExt*)s->ext;
								delete ext->item;
							}
						}
						gameExplorer->currentPipeline->stages[i] = s;
						for (int i = 0; i < 5; i++)
						{
							auto s = gameExplorer->currentPipeline->stages[i];
							if (s)
							{
								auto _ext = (StageExt*)s->ext;
								_ext->item = new QTreeWidgetItem;
								_ext->setItemText();
								ext->item->addChild(_ext->item);
							}
						}
					}

					ext->setChanged(true);

					return;
				}
			}
			break;
		}
		break;
	}
}

void WorldEditor::on_new_pipeline()
{
	auto list = QFileDialog::getOpenFileNames(this);
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto p = new tke::Pipeline;
		p->loadXML(list[i].toUtf8().data());
		p->ext = new PipelineExt(p);
		game.pipelines.push_back(p);

		if (gameExplorer)
		{
			auto ext = (PipelineExt*)p->ext;
			ext->item = new QTreeWidgetItem;
			ext->setItemText();
			gameExplorer->pipelinesItem->addChild(ext->item);

			for (int i = 0; i < 5; i++)
			{
				auto s = p->stages[i];
				if (s)
				{
					auto _ext = new StageExt(s);
					s->ext = ext;
					_ext->item = new QTreeWidgetItem;
					_ext->setItemText();
					ext->item->addChild(_ext->item);
				}
			}
		}
	}

	game.save();
}

void WorldEditor::on_save_selected_item()
{
	auto currentWindowType = getCurrentWindowType();
	switch (currentWindowType)
	{
	case WindowTypeGameExplorer:
		switch (gameExplorer->currentItemType)
		{
		case GameExplorer::ItemTypePipeline:
			save_pipeline(gameExplorer->currentPipeline);
			break;
		case GameExplorer::ItemTypeStage:
			save_stage(gameExplorer->currentStage);
			break;
		}
		break;
	case WindowTypeStageEditor:
		save_stage(currentStageEditor->stage);
		break;
	}
}

void WorldEditor::on_save_all()
{
	for (auto p : game.pipelines)
	{
		for (int i = 0; i < 5; i++)
		{
			auto s = p->stages[i];
			if (s) save_stage(s);
		}
		save_pipeline(p);
	}
}

void WorldEditor::on_open_in_file_explorer()
{
	int action;
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
		action = 0;
	else
		action = 1;
	std::string path;

	auto currentWindowType = getCurrentWindowType();
	switch (currentWindowType)
	{
	case WindowTypeGameExplorer:
		switch (gameExplorer->currentItemType)
		{
		case GameExplorer::ItemTypePipeline:
			path = gameExplorer->currentPipeline->filename;
			break;
		case GameExplorer::ItemTypeStage:
			path = gameExplorer->currentStage->parent->filepath + "/" + gameExplorer->currentStage->filename;
			break;
		default:
			action = -1;
		}
		break;
	case WindowTypeStageEditor:
		path = currentStageEditor->stage->parent->filepath + "/" + currentStageEditor->stage->filename;
		break;
	}

	if (action == 0)
	{
		ShellExecuteA(NULL, "open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else if (action == 1)
	{
		WinExec(("explorer /select," + std::experimental::filesystem::absolute(path).string()).c_str(), SW_SHOWNORMAL);
	}
}

void WorldEditor::on_remove()
{
	auto currentWindowType = getCurrentWindowType();
	switch (currentWindowType)
	{
	case WindowTypeGameExplorer:
		switch (gameExplorer->currentItemType)
		{
		case GameExplorer::ItemTypePipeline:
		{
			for (auto it = game.pipelines.begin(); it != game.pipelines.end(); it++)
			{
				if (*it == gameExplorer->currentPipeline)
				{
					game.pipelines.erase(it);
					break;
				}
			}

			delete gameExplorer->currentPipeline;

			game.save();

			gameExplorer->currentItemType = GameExplorer::ItemTypeNull;
		}
			break;
		case GameExplorer::ItemTypeStage:
		{
			auto s = gameExplorer->currentStage;

			auto p = s->parent;
			p->stages[tke::StageIndexByType(s->type)] = nullptr;

			auto ext = (PipelineExt*)p->ext;
			ext->changed = true;
			ext->item->setText(0, QString(p->filename.c_str()) + "*");

			delete s;

			gameExplorer->currentItemType = GameExplorer::ItemTypeNull;
		}
			break;
		}
		break;
	}

}

void WorldEditor::on_view_game_explorer()
{
	if (gameExplorer) return;
	gameExplorer = new GameExplorer(this);
	gameExplorer->setup();
	addDockWidget(Qt::TopDockWidgetArea, gameExplorer);
}

void WorldEditor::on_compile()
{
	auto currentWindowType = getCurrentWindowType();
	switch (currentWindowType)
	{
	case WindowTypeGameExplorer:
		switch (gameExplorer->currentItemType)
		{
		case GameExplorer::ItemTypePipeline:
		{
			for (int i = 0; i < 5; i++)
			{
				auto s = gameExplorer->currentPipeline->stages[i];
				if (s) compile_stage(s);
			}
			auto ext = (PipelineExt*)gameExplorer->currentPipeline->ext;
			ext->setChanged(true);
		}
			break;
		case GameExplorer::ItemTypeStage:
		{
			compile_stage(gameExplorer->currentStage);
			auto ext = (PipelineExt*)gameExplorer->currentStage->parent->ext;
			ext->setChanged(true);
		}
			break;
		}
		break;
	case WindowTypeStageEditor:
	{
		compile_stage(currentStageEditor->stage);
		auto ext = (PipelineExt*)currentStageEditor->stage->parent->ext;
		ext->setChanged(true);
	}
		break;
	}
}

void WorldEditor::save_pipeline(tke::Pipeline *p)
{
	auto ext = (PipelineExt*)p->ext;
	if (!ext->changed) return;
	p->saveXML();
	ext->setChanged(false);
}

void WorldEditor::save_stage(tke::Stage *stage)
{
	auto _ext = (StageExt*)stage->ext;

	if (!_ext->changed) return;

	if (_ext->editor)
		_ext->text = _ext->editor->edit->toPlainText().toUtf8().data();

	std::ofstream file(stage->parent->filepath + "/" + stage->filename);
	file.write(_ext->text.c_str(), _ext->text.size());

	_ext->setChanged(false);
}

void WorldEditor::compile_stage(tke::Stage *s)
{
	auto ext = (StageExt*)s->ext;

	save_stage(s);

	std::stringstream ss(ext->text);
	ext->outputText = "";

	std::string line;
	while (!ss.eof())
	{
		std::getline(ss, line);

		std::regex pat(R"(#include\s+\"([\w\.\\]+)\")");
		std::smatch sm;
		if (std::regex_search(line, sm, pat))
		{
			auto include = sm[1].str();
			tke::OnceFileBuffer file(s->parent->filepath + "/" + s->filepath + "/" + include);
			ext->outputText += file.data;
			ext->outputText += "\n";
		}
		else
		{
			ext->outputText += line + "\n";
		}
	}

	std::ofstream file("temp.glsl");
	file.write(ext->outputText.c_str(), ext->outputText.size());
	file.close();

	std::experimental::filesystem::path spv(s->parent->filepath + "/" + s->filename + ".spv");

	tke::exec("cmd", "/C glslangValidator my_glslValidator_config.conf -V temp.glsl -S " + tke::StageNameByType(s->type) + " -q -o " + spv.string() + " > output.txt");

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
