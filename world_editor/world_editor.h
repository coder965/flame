#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_world_editor.h"

class WorldEditor : public QMainWindow
{
	Q_OBJECT

public:
	WorldEditor(QWidget *parent = Q_NULLPTR);

private:
	Ui::WorldEditorClass ui;
};
