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
	void on_pipelineList_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev);

	void on_addPipeline_clicked();

	void on_removePipeline_clicked();

	void on_explorerPipeline_clicked();

	void on_addStage_clicked();

	void on_saveStage_clicked();

	void on_toSpv_clicked();

	void on_removeStage_clicked();

	void on_stageTab_currentChanged(int index);

	void on_explorerStage_clicked();

	void on_savePipeline_clicked();

	void on_find();

	void on_find_enter();

	void on_find_previous();

	void on_find_next();

protected:
	void keyPressEvent(QKeyEvent *k) override;

private:
	Ui::QtGuiApplicationClass ui;
};
