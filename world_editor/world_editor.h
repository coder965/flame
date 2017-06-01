#pragma once

#include <tuple>

#include <process.h>

#include <QtWidgets/QMainWindow>
#include "ui_world_editor.h"

#include <QWindow>
#include <QScrollArea>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QSplitter>
#include <QScrollArea>

#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <regex>

#include "../src/qLineNumberEdit.h"
#include "../src/qFindWidget.h"
#include "../src/core.h"
#include "../src/scene.h"
#include "../src/model.general.h"
#include "../src/model.file.h"
#include "../src/gui.h"
#include "../src/event.h"
#include "../src/image.file.h"

struct GameExplorer;
struct StageEditor;
struct MonitorWidget;
struct OutputWidget;

struct Pipeline;
struct Stage : tke::StageArchive
{
	Pipeline *parent = nullptr;

	bool changed = false;
	QTreeWidgetItem *item = nullptr;
	StageEditor *editor = nullptr;
	std::string text;
	void setItemText();
	void setChanged(bool _changed);
	Stage(Pipeline *_parent);
	~Stage();
};

struct Pipeline : tke::PipelineArchive
{
	Stage *stages[5] = {};
	bool changed = false;
	QTreeWidgetItem *item = nullptr;
	void setItemText();
	void setChanged(bool _changed);
	~Pipeline();
};

struct Renderer
{
	tke::Renderer *p;
	QTreeWidgetItem *item = nullptr;
	MonitorWidget *monitor = nullptr;
	void setItemText();
	~Renderer();
};

class WorldEditor : public QMainWindow
{
	Q_OBJECT

public:
	WorldEditor(QWidget *parent = Q_NULLPTR);

private slots:
	// menus
	void on_new_stage();
	void on_new_pipeline();
	void on_save_selected_item();
	void on_save_all();
	void on_open_in_file_explorer();
	void on_remove();
	void on_view_game_explorer();
	void on_view_output_widget();
	void on_compile();
private:
	Ui::WorldEditorClass ui;
	
	void save_pipeline(Pipeline *);
	void save_stage(Stage *);
	void compile_stage(Stage *);

protected:
	void keyPressEvent(QKeyEvent *k) override;
};

enum WindowType
{
	WindowTypeNull = -1,
	WindowTypeGameExplorer = 0,
	WindowTypeStageEditor = 1,
	WindowTypeMonitorWidget = 2,
	WindowTypeOutputWidget = 3
};

struct Game
{
	std::vector<Pipeline*> pipelines;
	std::vector<Renderer*> renderers;

	void load_pipelines();
	void save_pipelines();
	void load_renderers();
	void save_renderers();
};

struct GameExplorer : QDockWidget
{
	QTreeWidget *tree;
	QTreeWidgetItem *pipelinesItem;
	QTreeWidgetItem *renderersItem;
	QTreeWidgetItem *scenesItem;

	enum ItemType
	{
		ItemTypeNull = -1,
		ItemTypePipeline = 0,
		ItemTypeStage = 2,
		ItemTypeRenderer = 3
	};

	ItemType currentItemType = ItemTypeNull;

	Pipeline *currentPipeline = nullptr;
	Stage *currentStage = nullptr;
	Renderer *currentRenderer = nullptr;

	using QDockWidget::QDockWidget;
	void on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev);
	void on_item_dbClicked(QTreeWidgetItem *item, int column);
	void setup();
	void closeEvent(QCloseEvent *event) override;
};

struct StageEditor : QDockWidget
{
	Stage *stage;

	QLineNumberEdit *edit;
	QFindWidget *findWidget;

	using QDockWidget::QDockWidget;
	void edit_changed();
	void setup();
	void closeEvent(QCloseEvent *event) override;
};

struct MonitorWindow;
struct MonitorWidget : QDockWidget
{
	Renderer *renderer;
	MonitorWindow *window = nullptr;
	bool windowDead = false;

	using QDockWidget::QDockWidget;
	void setup();
	void closeEvent(QCloseEvent *event) override;
};

struct OutputWidget : QDockWidget
{
	QTextBrowser *text;

	using QDockWidget::QDockWidget;
	void setup();
	void closeEvent(QCloseEvent *event) override;
};
