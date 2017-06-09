#include "world_editor.h"

static WorldEditor *worldEditor = nullptr;

static GameExplorer *gameExplorer = nullptr;
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
		case WindowTypeMonitorWidget:
			currentMonitorWidget = (MonitorWidget*)dock;
			break;
		}
		return (WindowType)data->v;
	}
	return WindowTypeNull;
}

void Renderer::setItemText()
{
	if (item)
		item->setText(0, QString(filename.c_str()));
}

Renderer::~Renderer()
{
	delete p;
	delete item;
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
			auto r = new Renderer;
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
		node->attributes.emplace_back(new tke::NormalVariable("filename", &r->p->filename), std::string());
		at.children.push_back(node);
	}

	at.saveXML("pipelines.xml");
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
			currentRenderer->monitor->renderer = currentRenderer;
			currentRenderer->monitor->setup();
			worldEditor->addDockWidget(Qt::RightDockWidgetArea, currentRenderer->monitor);
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

float LinearDepthPerspective(float z, float depth_near, float depth_far)
{
	float a = (1.0 - depth_far / depth_near) * 0.5 / depth_far;
	float b = (1.0 + depth_far / depth_near) * 0.5 / depth_far;
	return 1.0 / (a * z + b);
}

struct MonitorWindow : tke::GuiWindow
{
	tke::Model *test_model;

	VkCommandBuffer cmd[2];

	tke::Renderer *renderer;

	MonitorWindow(tke::Renderer *_renderer)
		:GuiWindow(tke::resCx, tke::resCy, "TK Engine World Editor", false)
	{
		test_model = tke::createModel("brick.obj");
		tke::scene->addModel(test_model);

		auto obj = new tke::StaticObject(test_model);
		tke::scene->addObject(obj);

		auto lit = new tke::PointLight;
		lit->color = glm::vec3(1.f);
		lit->decayFactor = glm::vec3(0.5f, 0.f, 1.f);
		lit->setCoord(glm::vec3(0, 1, 0));
		tke::scene->addLight(lit);

		setRenderer(_renderer);

		for (int i = 0; i < 2; i++)
			cmd[i] = tke::commandPool.allocate();

		tke::scene->camera.setMode(tke::Camera::Mode::eTargeting);
		tke::scene->camera.setCoord(glm::vec3(0, 5, 0));
	}
	
	void setRenderer(tke::Renderer *_renderer)
	{
		renderer = _renderer;

		renderer->resource.setImage(images, "Window.Image");
		tke::ShaderMacro macro;
		macro.pipelineName = "Deferred.Pipeline";
		macro.stageType = tke::StageType::frag;
		macro.value = "#define USE_PBR\n";
		renderer->resource.shaderMacros.push_back(macro);

		renderer->setup();

		tke::scene->setResources(renderer);

		tke::needRedraw = true;
		tke::scene->needUpdataSky = true;
		tke::scene->needUpdateSampler = true;
	}

	void makeCmd()
	{
		for (int i = 0; i < 2; i++)
		{
			vkResetCommandBuffer(cmd[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			cmd[i] = tke::commandPool.allocate();

			tke::beginCommandBuffer(cmd[i]);

			auto objectDrawcall = renderer->findRenderPass("mrt")->findAction("1")->findDrawcall("1");
			objectDrawcall->indirect_count = tke::scene->staticIndirectCount;

			renderer->execute(cmd[i], i);

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
					tke::scene->camera.addAngAccrodingToScreen(distX, distY);
				}
			}
			else if (middlePressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					tke::scene->camera.moveAccrodingToScreen(distX, distY);
			}
			else if (rightPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					tke::scene->camera.scroll(mouseX - mousePrevX);
			}
		}
	}

	virtual void renderEvent() override
	{
		tke::scene->update();

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

struct MonitorWidgetData
{
	MonitorWindow **window;
	tke::Renderer *renderer;
	bool *dead;
};

struct QMyScrollArea : QScrollArea
{
	QSize sizeHint() const override
	{
		return QSize(tke::resCx, tke::resCy);
	}
};

void MonitorWidget::setup()
{
	setAttribute(Qt::WA_DeleteOnClose);

	MonitorWidgetData data;
	data.window = &window;
	renderer->p = new tke::Renderer;
	renderer->p->loadXML(renderer->filename);
	data.renderer = renderer->p;
	data.dead = &windowDead;

	_beginthread([](void *p){
		auto data = (MonitorWidgetData*)p;

		auto window = new MonitorWindow(data->renderer);

		*data->window = window;

		window->run(data->dead);
	}, 0, &data);
	while (!window) Sleep(100);

	auto qWnd = QWindow::fromWinId((unsigned int)window->hWnd);
	qWnd->setFlags(Qt::FramelessWindowHint);

	QWidget *container = QWidget::createWindowContainer(qWnd);
	container->setFixedWidth(tke::resCx);
	container->setFixedHeight(tke::resCy);

	auto scrollArea = new QMyScrollArea;
	scrollArea->setWidget(container);

	setUserData(0, new QMyUserData(WindowTypeOutputWidget));

	setWindowTitle(QString("Monitor - ") + renderer->p->filename.c_str());
	setWidget(scrollArea);
}

MonitorWidget::~MonitorWidget()
{
	SendMessage(window->hWnd, WM_CLOSE, 0, 0);
	delete renderer->p;
	renderer->p = nullptr;
	renderer->monitor = nullptr;
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


WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	worldEditor = this;

	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);

	game.load_renderers();

	connect(ui.action_save_selected_item, &QAction::triggered, this, &WorldEditor::on_save_selected_item);
	connect(ui.action_save_all, &QAction::triggered, this, &WorldEditor::on_save_all);
	connect(ui.action_open_in_file_explorer, &QAction::triggered, this, &WorldEditor::on_open_in_file_explorer);
	connect(ui.action_remove, &QAction::triggered, this, &WorldEditor::on_remove);
	connect(ui.action_view_output_widget, &QAction::triggered, this, &WorldEditor::on_view_output_widget);
	connect(ui.action_view_game_explorer, &QAction::triggered, this, &WorldEditor::on_view_game_explorer);
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
			delete r->p;
			r->p = new tke::Renderer;
			r->p->loadXML(r->filename);
			r->monitor->window->setRenderer(r->p);
			// resume the window
			r->monitor->window->state = tke::Window::State::eSinalToRun;
			while (r->monitor->window->state != tke::Window::State::eRunning) Sleep(100);
		}
	}
}
