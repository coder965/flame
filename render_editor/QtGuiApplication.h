#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtGuiApplication.h"

class QtGuiApplication : public QMainWindow
{
	Q_OBJECT

public:
	QtGuiApplication(QWidget *parent = Q_NULLPTR);

private slots:

void on_addButton_clicked();

void on_removeButton_clicked();

void on_listWidget_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev);

void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

void on_saveButton_clicked();

private:
	Ui::QtGuiApplicationClass ui;
};
