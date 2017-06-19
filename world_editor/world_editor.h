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
#include <QSlider>
#include <QLabel>
#include <QtOpenGL\QGLWidget>
#include <QTimer>

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

enum WindowType
{
	WindowTypeNull = -1,
	WindowTypeGameExplorer = 0,
	WindowTypeMonitorWidget = 1,
	WindowTypeOutputWidget = 2,
	WindowTypeAttributeWidget = 3
};

struct GameExplorer;
struct MonitorWidget;
struct OutputWidget;

struct RendererEditorStruct
{
	std::string filename;
	QTreeWidgetItem *item = nullptr;
	MonitorWidget *monitor = nullptr;

	void setItemText();
	~RendererEditorStruct();
};

struct ModelEditorStruct
{
	std::string filename;
	tke::Model *p = nullptr;
	QTreeWidgetItem *item = nullptr;
	MonitorWidget *monitor = nullptr;

	void setItemText();
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
	void on_view_attribute_widget();
	void on_update_changes();
private:
	Ui::WorldEditorClass ui;

	void update();

protected:
	void keyPressEvent(QKeyEvent *k) override;
};

struct Game
{
	std::vector<RendererEditorStruct*> renderers;
	std::vector<ModelEditorStruct*> models;

	void load();
	void save();
};

struct GameExplorer : QDockWidget
{
	QTreeWidget *tree;
	QTreeWidgetItem *renderersItem;
	QTreeWidgetItem *scenesItem;
	QTreeWidgetItem *modelsItem;

	enum ItemType
	{
		ItemTypeNull = -1,
		ItemTypeRenderer = 0,
		ItemTypeModel = 1
	};

	ItemType currentItemType = ItemTypeNull;

	RendererEditorStruct *currentRenderer = nullptr;
	ModelEditorStruct *currentModel = nullptr;

	GameExplorer(QWidget *_parent);
	~GameExplorer();
	void on_item_changed(QTreeWidgetItem *curr, QTreeWidgetItem *prev);
	void on_item_dbClicked(QTreeWidgetItem *item, int column);
};

struct MonitorWindow;
struct MonitorWidget : QDockWidget
{
	MonitorWidget **owner = nullptr;

	std::string renderer_filename;
	tke::Renderer *renderer = nullptr;
	tke::Scene *scene = nullptr;
	tke::Model *model = nullptr;

	MonitorWindow *window = nullptr;
	bool windowDead = false;

	MonitorWidget(QWidget *_parent, MonitorWidget **_owner, const std::string _renderer_filename, tke::Model *_model);
	~MonitorWidget();
};

struct OutputWidget : QDockWidget
{
	QTextBrowser *text;

	OutputWidget(QWidget *_parent);
	~OutputWidget();
};

struct AttributeWidget : tke::Observer, QDockWidget
{
	QTreeWidget *tree;

	AttributeWidget(QWidget *_parent);
	~AttributeWidget();
	void attachCurrentObject();
	virtual void listen(void *sender, tke::NotificationType type, void *newData) override;
};
