#include "world_editor.h"

tke::Model *test_model = nullptr;
std::string master_renderer_filename("../renderer/master.xml");

tke::Object *currentObject = nullptr;

static WorldEditor *worldEditor = nullptr;

static GameExplorer *gameExplorer = nullptr;
static MonitorWidget *currentMonitorWidget = nullptr;
static OutputWidget *outputWidget = nullptr;
static ObjectController *objectController = nullptr;

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
		case WindowTypeMonitorWidget:
			currentMonitorWidget = (MonitorWidget*)dock;
			break;
		}
		return (WindowType)data->v;
	}
	return WindowTypeNull;
}

void RendererEditorStruct::setItemText()
{
	if (item)
		item->setText(0, QString(filename.c_str()));
}

RendererEditorStruct::~RendererEditorStruct()
{
	delete item;
}

void ModelEditorStruct::setItemText()
{
	if (item)
		item->setText(0, QString(filename.c_str()));
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
			auto r = new RendererEditorStruct;
			r->filename = a->second;
			renderers.push_back(r);
		}
	}
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

void Game::load_models()
{
	tke::AttributeTree at("data");
	at.loadXML("models.xml");
	for (auto c : at.children)
	{
		if (c->name == "model")
		{
			auto a = c->firstAttribute("filename");
			auto m = new ModelEditorStruct;
			m->filename = a->second;
			m->p = tke::createModel(a->second);
			models.push_back(m);
		}
	}
}

static Game game;

void GameExplorer::on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev)
{
	for (auto r : game.renderers)
	{
		if (r->item == curr)
		{
			currentItemType = ItemTypeRenderer;
			currentRenderer = r;
			return;
		}
	}
	for (auto m : game.models)
	{
		if (m->item == curr)
		{
			currentItemType = ItemTypeModel;
			currentModel = m;
			return;
		}
	}
	currentItemType = ItemTypeNull;
}

void GameExplorer::on_item_dbClicked(QTreeWidgetItem *item, int column)
{
	switch (currentItemType)
	{
	case ItemTypeRenderer:
	{
		if (!currentRenderer->monitor)
		{
			currentRenderer->monitor = new MonitorWidget(worldEditor);
			currentRenderer->monitor->renderer_filename = currentRenderer->filename;
			currentRenderer->monitor->model = test_model;
			currentRenderer->monitor->setup(&currentRenderer->monitor);
			worldEditor->addDockWidget(Qt::RightDockWidgetArea, currentRenderer->monitor);
		}
	}
		break;
	case ItemTypeModel:
	{
		if (!currentModel->monitor)
		{
			currentModel->monitor = new MonitorWidget(worldEditor);
			currentModel->monitor->renderer_filename = master_renderer_filename;
			currentModel->monitor->model = currentModel->p;
			currentModel->monitor->setup(&currentModel->monitor);
			worldEditor->addDockWidget(Qt::RightDockWidgetArea, currentModel->monitor);
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

	renderersItem = new QTreeWidgetItem;
	renderersItem->setText(0, "Renderers");
	for (auto r : game.renderers)
	{
		r->item = new QTreeWidgetItem;
		r->setItemText();
		renderersItem->addChild(r->item);
	}
	tree->addTopLevelItem(renderersItem);

	scenesItem = new QTreeWidgetItem;
	scenesItem->setText(0, "Scenes");
	tree->addTopLevelItem(scenesItem);

	modelsItem = new QTreeWidgetItem;
	modelsItem->setText(0, "Models");
	for (auto m : game.models)
	{
		m->item = new QTreeWidgetItem;
		m->setItemText();
		modelsItem->addChild(m->item);
	}
	tree->addTopLevelItem(modelsItem);

	setWidget(tree);
	connect(tree, &QTreeWidget::currentItemChanged, this, &GameExplorer::on_item_changed);
	connect(tree, &QTreeWidget::itemDoubleClicked, this, &GameExplorer::on_item_dbClicked);
}

void GameExplorer::closeEvent(QCloseEvent *event)
{
	for (auto r : game.renderers)
		r->item = nullptr;
	delete gameExplorer;
	gameExplorer = nullptr;
}

struct MonitorWindow : tke::GuiWindow
{
	MonitorWidget *widget;

	tke::Object *obj;
	tke::Light *lit;

	VkCommandBuffer cmd[2];

	MonitorWindow(MonitorWidget *_widget)
		:GuiWindow(tke::resCx, tke::resCy, "TK Engine World Editor", false)
	{
		widget = _widget;

		obj = new tke::Object(widget->model);
		widget->scene->addObject(obj);

		lit = new tke::Light(tke::LightTypePoint);
		lit->color = glm::vec3(1.f);
		lit->setCoord(0.f, 1.f, 0.f);
		widget->scene->addLight(lit);

		setRenderer();

		for (int i = 0; i < 2; i++)
			cmd[i] = tke::commandPool.allocate();

		widget->scene->camera.setMode(tke::CameraModeTargeting);
		widget->scene->camera.setCoord(0.f, 5.f, 0.f);
	}
	
	void setRenderer()
	{
		auto renderer = widget->renderer;

		renderer->resource.setImage(images, "Window.Image");
		tke::ShaderMacro macro;
		macro.pipelineName = "Deferred.Pipeline";
		macro.stageType = tke::StageType::frag;
		macro.value = "#define USE_PBR\n";
		renderer->resource.shaderMacros.push_back(macro);

		renderer->setup();

		tke::setMasterRenderer(renderer);
		widget->scene->setRenderer(renderer);

		tke::needRedraw = true;
		tke::needUpdateTexture = true;
		widget->scene->needUpdateSky = true;
	}

	void makeCmd()
	{
		for (int i = 0; i < 2; i++)
		{
			vkResetCommandBuffer(cmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			cmd[i] = tke::commandPool.allocate();

			tke::beginCommandBuffer(cmd[i]);

			widget->renderer->execute(cmd[i], i);

			vkCmdSetEvent(cmd[i], renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

			vkEndCommandBuffer(cmd[i]);
		}
	}

	virtual void mouseEvent() override
	{
		if (tke::uiAcceptedMouse)
			return;

		float distX = mouseX - mousePrevX;
		float distY = mouseY - mousePrevY;

		if (distX != 0 || distY != 0)
		{
			distX /= tke::resCx;
			distY /= tke::resCy;
			if (leftPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
				{
					widget->scene->camera.addAngAccrodingToScreen(distX, distY);
				}
			}
			else if (middlePressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					widget->scene->camera.moveAccrodingToScreen(distX, distY);
			}
			else if (rightPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					widget->scene->camera.scroll(mouseX - mousePrevX);
			}
		}
	}

	virtual void renderEvent() override
	{
		widget->scene->update();

		if (tke::needRedraw)
		{
			makeCmd();
			tke::needRedraw = false;
		}

		beginFrame();

		beginUi();
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", getFPS());
		ImGui::End();
		endUi();

		VkCommandBuffer cmds[2] = { cmd[imageIndex], uiCmd };
		tke::graphicsQueue.submitFence(imageAvailable, 2, cmds, frameDone);

		endFrame();
	}
};

struct QMyScrollArea : QScrollArea
{
	QSize sizeHint() const override
	{
		return QSize(tke::resCx, tke::resCy);
	}
};

void MonitorWidget::setup(MonitorWidget **_owner)
{
	owner = _owner;

	setAttribute(Qt::WA_DeleteOnClose);

	renderer = new tke::Renderer;
	renderer->loadXML(renderer_filename);
	scene = new tke::Scene;

	_beginthread([](void *_p){
		auto p = (MonitorWidget*)_p;
		p->window = new MonitorWindow(p);
		p->window->run(&p->windowDead);
	}, 0, this);
	while (!window) Sleep(100);

	auto qWnd = QWindow::fromWinId((unsigned int)window->hWnd);
	qWnd->setFlags(Qt::FramelessWindowHint);

	QWidget *container = QWidget::createWindowContainer(qWnd);
	container->setFixedWidth(tke::resCx);
	container->setFixedHeight(tke::resCy);

	auto scrollArea = new QMyScrollArea;
	scrollArea->setWidget(container);

	setUserData(0, new QMyUserData(WindowTypeOutputWidget));

	setWindowTitle(QString("Monitor - ") + renderer->filename.c_str());
	setWidget(scrollArea);
}

void MonitorWidget::closeEvent(QCloseEvent *event)
{
	SendMessage(window->hWnd, WM_CLOSE, 0, 0);

	delete renderer;

	delete scene;

	*owner = nullptr;
}

void OutputWidget::setup()
{
	setAttribute(Qt::WA_DeleteOnClose);

	setUserData(0, new QMyUserData(WindowTypeOutputWidget));

	setWindowTitle("Output");

	text = new QTextBrowser;
	text->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);

	setWidget(text);
}

OutputWidget::~OutputWidget()
{
	outputWidget = nullptr;
}

void ObjectController::attachCurrentObject()
{

}

void ObjectController::setup()
{
	auto spliter = new QSplitter(Qt::Vertical);
	tree = new QTreeWidget;
	spliter->addWidget(tree);
	auto group = new QGroupBox;
	auto exSlider = new QSlider(Qt::Horizontal, group);
	auto eySlider = new QSlider(Qt::Horizontal, group);
	spliter->addWidget(group);
	setWidget(spliter);

	attachCurrentObject();
}

ObjectController::~ObjectController()
{

}

WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	test_model = tke::createModel("brick.obj");

	worldEditor = this;

	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);

	game.load_renderers();
	game.load_models();

	connect(ui.action_save_selected_item, &QAction::triggered, this, &WorldEditor::on_save_selected_item);
	connect(ui.action_save_all, &QAction::triggered, this, &WorldEditor::on_save_all);
	connect(ui.action_open_in_file_explorer, &QAction::triggered, this, &WorldEditor::on_open_in_file_explorer);
	connect(ui.action_remove, &QAction::triggered, this, &WorldEditor::on_remove);
	connect(ui.action_view_game_explorer, &QAction::triggered, this, &WorldEditor::on_view_game_explorer);
	connect(ui.action_view_output_widget, &QAction::triggered, this, &WorldEditor::on_view_output_widget);
	connect(ui.action_view_bone_controller, &QAction::triggered, this, &WorldEditor::on_view_bone_controller);
	connect(ui.action_update_changes, &QAction::triggered, this, &WorldEditor::on_update_changes);

	on_view_output_widget();
	on_view_game_explorer();
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
}

void WorldEditor::on_save_selected_item()
{
	auto currentWindowType = getCurrentWindowType();
	switch (currentWindowType)
	{
	case WindowTypeGameExplorer:
		switch (gameExplorer->currentItemType)
		{
		}
		break;
	}
}

void WorldEditor::on_save_all()
{
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
		default:
			action = -1;
		}
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

void WorldEditor::on_view_bone_controller()
{
	if (objectController) return;
	objectController = new ObjectController(this);
	objectController->setup();
	addDockWidget(Qt::RightDockWidgetArea, objectController);
}

void WorldEditor::on_update_changes()
{
	tke::exec("cmd", "/C cd ..\\shader_compiler & Debug\\shader_compiler.exe > ..\\world_editor\\output.txt");

	if (outputWidget)
	{
		tke::OnceFileBuffer outputFile("output.txt");
		outputWidget->text->setPlainText(outputFile.data);
	}
	
	for (auto r : game.renderers)
	{
		if (r->monitor)
		{
			r->monitor->window->state = tke::Window::State::eSinalToPause;
			while (r->monitor->window->state != tke::Window::State::ePausing) Sleep(100);
			// wait for window pausing
			delete r->monitor->renderer;
			r->monitor->renderer = new tke::Renderer;
			r->monitor->renderer->loadXML(r->filename);
			r->monitor->window->setRenderer();
			// resume the window
			r->monitor->window->state = tke::Window::State::eSinalToRun;
			while (r->monitor->window->state != tke::Window::State::eRunning) Sleep(100);
		}
	}
}
