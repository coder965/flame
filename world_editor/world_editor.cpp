#include "world_editor.h"

static WorldEditor *worldEditor = nullptr;

static GameExplorer *gameExplorer = nullptr;
static StageEditor *currentStageEditor = nullptr;
static MonitorWidget *currentMonitorWidget = nullptr;
static OutputWidget *outputWidget = nullptr;

struct QMyUserData : QObjectUserData
{
	int v;
	QMyUserData(int _v)
	{
		v = _v;
	}
};

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
		auto data = (QMyUserData*)dock->userData(0);
		switch (data->v)
		{
		case WindowTypeStageEditor:
			currentStageEditor = (StageEditor*)dock;
			break;
		case WindowTypeMonitorWidget:
			currentMonitorWidget = (MonitorWidget*)dock;
			break;
		}
		return (WindowType)data->v;
	}
	return WindowTypeNull;
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

void RendererExt::setItemText()
{
	if (item)
		item->setText(0, QString(p->filename.c_str()));
}

RendererExt::RendererExt(tke::Renderer *_p)
{
	p = _p;
}

RendererExt::~RendererExt()
{
	delete item;
}

void Game::load_pipelines()
{
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

void Game::save_pipelines()
{
	tke::AttributeTree at("data");

	for (auto p : pipelines)
	{
		auto node = new tke::AttributeTreeNode("pipeline");
		node->attributes.emplace_back(new tke::NormalVariable("filename", &p->filename), std::string());
		at.children.push_back(node);
	}

	at.saveXML("pipelines.xml");
}

void Game::save_renderers()
{
	tke::AttributeTree at("data");

	for (auto r : renderers)
	{
		auto node = new tke::AttributeTreeNode("renderer");
		node->attributes.emplace_back(new tke::NormalVariable("filename", &r->filename), std::string());
		at.children.push_back(node);
	}

	at.saveXML("pipelines.xml");
}

void Game::load_renderers()
{
	tke::AttributeTree at("data");
	at.loadXML("renderers.xml");
	for (auto c : at.children)
	{
		if (c->name == "renderer")
		{
			auto a = c->firstAttribute("filename");
			auto r = new tke::Renderer;
			r->loadXML(a.second);
			r->ext = new RendererExt(r);
			renderers.push_back(r);
		}
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
	for (auto r : game.renderers)
	{
		auto ext = (RendererExt*)r->ext;
		if (ext->item == curr)
		{
			currentItemType = ItemTypeRenderer;
			currentRenderer = r;
			return;
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
			worldEditor->addDockWidget(Qt::RightDockWidgetArea, ext->editor);
		}
	}
		break;
	case ItemTypeRenderer:
	{
		auto ext = (RendererExt*)currentRenderer->ext;
		if (!ext->monitor)
		{
			ext->monitor = new MonitorWidget(worldEditor);
			ext->monitor->renderer = currentRenderer;
			ext->monitor->setup();
			worldEditor->addDockWidget(Qt::RightDockWidgetArea, ext->monitor);
		}
	}
		break;
	}
}

void GameExplorer::setup()
{
	setUserData(0, new QMyUserData(WindowTypeGameExplorer));

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
	for (auto r : game.renderers)
	{
		auto ext = (RendererExt*)r->ext;
		ext->item = new QTreeWidgetItem;
		ext->setItemText();
		renderersItem->addChild(ext->item);
	}
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
			if (s)
			{
				auto _ext = (StageExt*)s->ext;
				_ext->item = nullptr;
			}
		}
	}
	for (auto r : game.renderers)
	{
		auto ext = (RendererExt*)r->ext;
		ext->item = nullptr;
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
	setUserData(0, new QMyUserData(WindowTypeStageEditor));

	auto _ext = (StageExt*)stage->ext;

	setWindowTitle(QString("Stage Editor - ") + stage->parent->name.c_str() + ":" + tke::StageNameByType(stage->type).c_str());

	edit = new QLineNumberEdit;
	edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
	edit->setPlainText(_ext->text.c_str());
	connect(edit, &QPlainTextEdit::textChanged, this, &StageEditor::edit_changed);
	//ext->editor->findWidget = new QFindWidget;
	//ext->editor->stageTab->setCornerWidget(ext->editor->findWidget->group);

	setWidget(edit);
}

void StageEditor::closeEvent(QCloseEvent *event)
{
	auto ext = (StageExt*)stage->ext;
	delete ext->editor;
	ext->editor = nullptr;
}

tke::UniformBuffer *pasteBuffer = nullptr;
struct MonitorWindow : tke::GuiWindow
{
	VkCommandBuffer progressCmd[2];

	tke::Image *titleImage;

	tke::Renderer *progressRenderer;

	void init()
	{
		progressRenderer = new tke::Renderer();
		progressRenderer->loadXML("../renderer/progress.xml");

		titleImage = tke::createImage("../misc/title.jpg", true, false);
		progressRenderer->resource.setImage(titleImage, "Paste.Texture");

		progressRenderer->resource.setImage(image, "Window.Image");

		progressRenderer->setup();

		for (int i = 0; i < 2; i++)
		{
			progressCmd[i] = tke::vk::commandPool.allocate();
			tke::vk::beginCommandBuffer(progressCmd[i]);
			progressRenderer->execute(progressCmd[i], i);
			vkCmdSetEvent(progressCmd[i], tke::renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			vkEndCommandBuffer(progressCmd[i]);
		}

		pasteBuffer = (tke::UniformBuffer*)progressRenderer->resource.getBuffer("Paste.UniformBuffer");

		auto list = new tke::EventList;
		tke::Event e0;
		e0.tickFunc = [](int _t) {
			if (_t < 500)
			{
				float alpha = _t / 500.f;
				pasteBuffer->update(&alpha, &tke::stagingBuffer);
			}
		};
		e0.duration = 1500;
		list->events.push_back(e0);
		tke::addEventList(list);
	}

	virtual void drawUi() override
	{
		ImGui::Begin("Progress", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::TextUnformatted(tke::majorProgressText().c_str());
		ImGui::TextUnformatted(tke::minorProgressText().c_str());
		ImGui::End();
	}

	virtual void renderEvent() override
	{
		tke::beginFrame();

		VkCommandBuffer cmds[2] = { progressCmd[tke::imageIndex], tke::uiCmd[tke::imageIndex] };
		tke::vk::queueSubmitFence(tke::imageAvailable, 2, cmds, tke::frameDone);

		tke::endFrame();
	}

	~MonitorWindow()
	{

	}
};

static void _monitor_thread(void *p)
{
	auto window = new MonitorWindow;
	window->create(tke::resCx, tke::resCy, "TK Engine World Editor", false);
	window->init();

	*(MonitorWindow**)p = window;

	tke::mainLoop(window);
}

void MonitorWidget::setup()
{
	_beginthread(_monitor_thread, 0, &monitorWindow);
	while (!monitorWindow) Sleep(100);

	QWindow *window = QWindow::fromWinId((unsigned int)monitorWindow->hWnd);
	window->setFlags(Qt::FramelessWindowHint);

	QWidget *widget = QWidget::createWindowContainer(window);
	//widget->setFixedWidth(1600);
	//widget->setFixedHeight(900);

	//ui.scrollArea->setWidget(widget);

	setUserData(0, new QMyUserData(WindowTypeOutputWidget));

	setWindowTitle(QString("Monitor - ") + renderer->filename.c_str());
	setWidget(widget);
}

void MonitorWidget::closeEvent(QCloseEvent *event)
{


	auto ext = (RendererExt*)renderer->ext;
	delete ext->monitor;
	ext->monitor = nullptr;
}

void OutputWidget::setup()
{
	setUserData(0, new QMyUserData(WindowTypeOutputWidget));

	setWindowTitle("Output");

	text = new QTextBrowser;
	text->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);

	setWidget(text);
}

void OutputWidget::closeEvent(QCloseEvent *event)
{
	delete outputWidget;
	outputWidget = nullptr;
}

#include "stage_editor_yy_def.h"
extern "C" {
	extern FILE *stage_editor_yyin;
	extern int stage_editor_yylex();
	int stage_editor_yylex_destroy();
	extern char *stage_editor_yytext;
}

WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	worldEditor = this;

	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);

	game.load_pipelines();
	game.load_renderers();

	connect(ui.action_new_stage, &QAction::triggered, this, &WorldEditor::on_new_stage);
	connect(ui.action_new_pipeline, &QAction::triggered, this, &WorldEditor::on_new_pipeline);
	connect(ui.action_save_selected_item, &QAction::triggered, this, &WorldEditor::on_save_selected_item);
	connect(ui.action_save_all, &QAction::triggered, this, &WorldEditor::on_save_all);
	connect(ui.action_open_in_file_explorer, &QAction::triggered, this, &WorldEditor::on_open_in_file_explorer);
	connect(ui.action_remove, &QAction::triggered, this, &WorldEditor::on_remove);
	connect(ui.action_view_output_widget, &QAction::triggered, this, &WorldEditor::on_view_output_widget);
	connect(ui.action_view_game_explorer, &QAction::triggered, this, &WorldEditor::on_view_game_explorer);
	connect(ui.action_compile, &QAction::triggered, this, &WorldEditor::on_compile);
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

			auto i = tke::StageIndexByType(type);
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

	game.save_pipelines();
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

			game.save_pipelines();

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
	addDockWidget(Qt::LeftDockWidgetArea, gameExplorer);
}

void WorldEditor::on_view_output_widget()
{
	if (outputWidget) return;
	outputWidget = new OutputWidget(this);
	outputWidget->setup();
	addDockWidget(Qt::BottomDockWidgetArea, outputWidget);
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
			for (int i = 0; i < 5; i++)
			{
				auto s = gameExplorer->currentPipeline->stages[i];
				if (s) compile_stage(s);
			}
			break;
		case GameExplorer::ItemTypeStage:
			compile_stage(gameExplorer->currentStage);
			break;
		}
		break;
	case WindowTypeStageEditor:
		compile_stage(currentStageEditor->stage);
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
	auto _ext = (StageExt*)s->ext;

	save_stage(s);

	std::stringstream ss(_ext->text);
	std::string stageText = "";

	std::vector<std::tuple<int, int, int>> includeFileDatas;
	std::string line;
	int lineNum = 0;
	int fullLineNum = 0;
	while (!ss.eof())
	{
		std::getline(ss, line);

		std::regex pat(R"(#include\s+\"([\w\.\\]+)\")");
		std::smatch sm;
		if (std::regex_search(line, sm, pat))
		{
			tke::OnceFileBuffer file(s->parent->filepath + "/" + s->filepath + "/" + sm[1].str());
			stageText += file.data;
			stageText += "\n";

			auto includeFileLineNum = tke::lineNumber(file.data);
			includeFileDatas.push_back({ lineNum, fullLineNum, includeFileLineNum });

			fullLineNum += includeFileLineNum;
		}
		else
		{
			stageText += line + "\n";

			fullLineNum += 1;
		}

		lineNum++;
	}

	std::ofstream file("temp.glsl");
	file.write(stageText.c_str(), stageText.size());
	file.close();

	std::experimental::filesystem::path spv(s->parent->filepath + "/" + s->filename + ".spv");

	tke::exec("cmd", "/C glslangValidator my_glslValidator_config.conf -V temp.glsl -S " + tke::StageNameByType(s->type) + " -q -o " + spv.string() + " > output.txt");

	std::string output;

	{
		tke::OnceFileBuffer outputFile("output.txt");
		output = outputFile.data;
		output += "\n";
	}

	bool error = false;

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

		stage_editor_yylex_destroy();

		stage_editor_yyin = fopen("output.txt", "rb");
		int token = stage_editor_yylex();
		std::string last_string;
		while (token)
		{
			switch (token)
			{
			case STAGE_EDITOR_YY_ERROR:
			{
				error = true;
				token = stage_editor_yylex();
				if (token == STAGE_EDITOR_YY_VALUE)
				{
					auto n = std::stoi(stage_editor_yytext);

					for (auto it = includeFileDatas.rbegin(); it != includeFileDatas.rend(); it++)
					{
						if (n > std::get<1>(*it) + std::get<2>(*it))
						{
							n = n - std::get<1>(*it) + std::get<0>(*it) - std::get<2>(*it) - 1;
							break;
						}
						else if (n > std::get<1>(*it))
						{
							n = std::get<0>(*it);
							break;
						}
					}

					output += std::string("Error, line num:") + std::to_string(n) + "\n";
				}
			}
				break;
			case STAGE_EDITOR_YY_COLON:
				if (currentReflectionType != eNull)
				{
					if (currentReflection.name != "") reflections.add(currentReflection);
					currentReflection.reflectionType = currentReflectionType;
					currentReflection.name = last_string;
					last_string = "";
				}
				break;
			case STAGE_EDITOR_YY_IDENTIFIER:
			{
				std::string string(stage_editor_yytext);
				last_string = string;
			}
				break;
			case STAGE_EDITOR_YY_VALUE:
			{
				std::string string(stage_editor_yytext);
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
			case STAGE_EDITOR_YY_UR_MK:
				currentReflectionType = eUniform;
				break;
			case STAGE_EDITOR_YY_UBR_MK:
				currentReflectionType = eUniformBlock;
				break;
			case STAGE_EDITOR_YY_VAR_MK:
				currentReflectionType = eVertexAttribute;
				break;
			}
			token = stage_editor_yylex();
		}
		if (currentReflection.name != "") reflections.add(currentReflection);
		fclose(stage_editor_yyin);
		stage_editor_yyin = NULL;

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

	if (!error)
	{
		auto ext = (PipelineExt*)s->parent->ext;
		ext->setChanged(true);
	}

	if (outputWidget)
	{
		auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::stringstream ss;
		ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");

		output = (ss.str() + "\n" +
			"Warnning:push constants in different stages must be merged, or else they would not reflect properly.\n\n" +
			output);

		outputWidget->text->setPlainText(output.c_str());
	}
}
