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
#include "../src/gui.h"
#include "../src/event.h"

struct GameExplorer;
struct MonitorWidget;
struct OutputWidget;

struct Renderer
{
	std::string filename;
	tke::Renderer *p = nullptr;
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
	void on_save_selected_item();
	void on_save_all();
	void on_open_in_file_explorer();
	void on_remove();
	void on_view_game_explorer();
	void on_view_output_widget();
	void on_update_changes();
private:
	Ui::WorldEditorClass ui;

protected:
	void keyPressEvent(QKeyEvent *k) override;
};

enum WindowType
{
	WindowTypeNull = -1,
	WindowTypeGameExplorer = 0,
	WindowTypeMonitorWidget = 1,
	WindowTypeOutputWidget = 2
};

struct Game
{
	std::vector<Renderer*> renderers;

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
		ItemTypeRenderer = 0
	};

	ItemType currentItemType = ItemTypeNull;

	Renderer *currentRenderer = nullptr;

	using QDockWidget::QDockWidget;
	void on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev);
	void on_item_dbClicked(QTreeWidgetItem *item, int column);
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
	~MonitorWidget();
};

struct OutputWidget : QDockWidget
{
	QTextBrowser *text;

	using QDockWidget::QDockWidget;
	void setup();
	~OutputWidget();
};
