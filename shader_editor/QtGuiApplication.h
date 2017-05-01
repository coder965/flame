#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtGuiApplication.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

#include <QDir>

#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QTextBlock>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPainter>

class QtGuiApplication : public QMainWindow
{
	Q_OBJECT

public:
	QtGuiApplication(QWidget *parent = Q_NULLPTR);

private slots:
	void on_pipelineTree_currentItemChanged(QTreeWidgetItem *_curr, QTreeWidgetItem *_prev);

	void on_addPipelineToolButton_clicked();

	void on_removePipelineToolButton_clicked();

	void on_explorerPipelineFileToolButton_clicked();

	void on_addStageToolButton_clicked();

	void on_saveStageToolButton_clicked();

	void on_toSpvToolButton_clicked();

	void on_removeStageToolButton_clicked();

	void on_stageTabWidget_tabBarClicked(int index);

	void on_explorerStageFileToolButton_clicked();

	void on_savePipelineToolButton_clicked();

	void on_find();

	void on_find_enter();

	void on_find_previous();

	void on_find_next();

	void on_stageTabWidget_currentChanged(int index);

protected:
	void keyPressEvent(QKeyEvent *k) override;

private:
	Ui::QtGuiApplicationClass ui;
};
