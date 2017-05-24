#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_world_editor.h"

#include <qwindow.h>
#include <qscrollarea.h>
#include <qplaintextedit.h>
#include <qpainter.h>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

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
	void on_pipelineList_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev);
	void on_vertEdit_textChanged();
	void on_tescEdit_textChanged();
	void on_teseEdit_textChanged();
	void on_geomEdit_textChanged();
	void on_fragEdit_textChanged();
	void on_addPipeline_clicked();
	void on_removePipeline_clicked();
	void on_savePipeline_clicked();
	void on_explorerPipeline_clicked();
	void on_stageTab_currentChanged(int index);
	void on_addStage_clicked();
	void on_removeStage_clicked();
	void on_saveStage_clicked();
	void on_toSpv_clicked();
	void on_explorerStage_clicked();
private:
	Ui::WorldEditorClass ui;
	
	void stage_edit_changed(int i);
	void save_pipeline(tke::Pipeline *);
	void save_stage(int i);
	void stage_to_spv(int i);

protected:
	void keyPressEvent(QKeyEvent *k) override;
};
