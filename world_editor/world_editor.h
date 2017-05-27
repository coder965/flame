#pragma once

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

#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <regex>

#include "../src/window.h"
#include "../src/qLineNumberEdit.h"
#include "../src/qFindWidget.h"

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
struct Monitor;

enum WindowType
{
	WindowTypeNull = -1,
	WindowTypeGameExplorer = 0,
	WindowTypeStageEditor = 1,
	WindowTypeMonitor = 2
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
	std::string outputText;
	std::string compileText;
	void setItemText();
	void setChanged(bool _changed);
	StageExt(tke::Stage *_p);
	~StageExt();
};

struct Game
{
	std::vector<tke::Pipeline*> pipelines;
	std::vector<tke::Renderer*> renderers;

	void load();
	void save();
};

struct GameExplorer : QDockWidget
{
	QTreeWidget *tree;
	QTreeWidgetItem *pipelinesItem;
	QTreeWidgetItem *renderersItem;
	QTreeWidgetItem *scenesItem;
	using QDockWidget::QDockWidget;

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

	void on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev);
	void on_item_dbClicked(QTreeWidgetItem *item, int column);
	void setup();
	void closeEvent(QCloseEvent *event) override;
};

struct StageEditor : QDockWidget
{
	tke::Stage *stage;
	using QDockWidget::QDockWidget;

	QLineNumberEdit *edit;
	QFindWidget *findWidget;
	QLineNumberEdit *outputText;
	QTextBrowser *compileText;

	void edit_changed();
	void setup();
	void closeEvent(QCloseEvent *event) override;
};
