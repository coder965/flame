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
#include "../src/gui.h"
#include "../src/event.h"
#include "../src/image.file.h"

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
	
	void save_pipeline(tke::Pipeline *);
	void save_stage(tke::Stage *);
	void compile_stage(tke::Stage *);

protected:
	void keyPressEvent(QKeyEvent *k) override;
};

struct GameExplorer;
struct StageEditor;
struct MonitorWidget;
struct OutputWidget;

enum WindowType
{
	WindowTypeNull = -1,
	WindowTypeGameExplorer = 0,
	WindowTypeStageEditor = 1,
	WindowTypeMonitorWidget = 2,
	WindowTypeOutputWidget = 3
};

struct PipelineExt : tke::ExtType
{
	tke::Pipeline *p;
	bool changed = false;
	QTreeWidgetItem *item = nullptr;
	void setItemText();
	void setChanged(bool _changed);
	PipelineExt(tke::Pipeline *_p);
	~PipelineExt();
};

struct StageExt : tke::ExtType
{
	tke::Stage *p;
	bool changed = false;
	QTreeWidgetItem *item = nullptr;
	StageEditor *editor = nullptr;
	std::string text;
	void setItemText();
	void setChanged(bool _changed);
	StageExt(tke::Stage *_p);
	~StageExt();
};

struct RendererExt : tke::ExtType 
{
	tke::Renderer *p;
	QTreeWidgetItem *item = nullptr;
	MonitorWidget *monitor = nullptr;
	void setItemText();
	RendererExt(tke::Renderer *_p);
	~RendererExt();
};

struct Game
{
	std::vector<tke::Pipeline*> pipelines;
	std::vector<tke::Renderer*> renderers;

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

	tke::Pipeline *currentPipeline = nullptr;
	tke::Stage *currentStage = nullptr;
	tke::Renderer *currentRenderer = nullptr;

	using QDockWidget::QDockWidget;
	void on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev);
	void on_item_dbClicked(QTreeWidgetItem *item, int column);
	void setup();
	void closeEvent(QCloseEvent *event) override;
};

struct StageEditor : QDockWidget
{
	tke::Stage *stage;

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
	tke::Renderer *renderer;
	MonitorWindow *monitorWindow = nullptr;

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
