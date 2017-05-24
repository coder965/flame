#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_world_editor.h"

class WorldEditor : public QMainWindow
{
	Q_OBJECT

public:
	WorldEditor(QWidget *parent = Q_NULLPTR);

private slots:
	void on_pipelineList_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev);

private:
	Ui::WorldEditorClass ui;

protected:
	void keyPressEvent(QKeyEvent *k) override;
};
