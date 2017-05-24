#include "world_editor.h"

#include <qwindow.h>
#include <qscrollarea.h>
#include <qplaintextedit.h>
#include <qpainter.h>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTreeWidgetItem>
#include <QTextBlock>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPainter>

#include <Windows.h>

#include "../src/window.h"
#include "../src/qLineNumberEdit.hpp"
#include "../src/qFindWidget.hpp"

namespace PipelineEditor
{
	struct PipelineExt : tke::ExtType
	{
		QListWidgetItem *item = nullptr;
		PipelineExt()
		{
			item = new QListWidgetItem;
		}
		~PipelineExt()
		{
			delete item;
		}
	};

	struct StageExt : tke::ExtType
	{
		std::string text;
	};

	std::vector<tke::Pipeline*> pipelines;
	tke::Pipeline *currentPipeline = nullptr;
	QLineNumberEdit *stageEdit[5];
	QLineNumberEdit *outputText;
	QTextBrowser *compileText;
	QFindWidget *findWidget;

	void saveDataXml()
	{
		tke::AttributeTree at("data");

		for (auto pipeline : pipelines)
		{
			auto node = new tke::AttributeTreeNode("pipeline");
			node->attributes.emplace_back(new tke::NormalVariable("filename", &pipeline->filename), std::string());
		}

		at.saveXML("pipelines.xml");
	}
}

WorldEditor::WorldEditor(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);

	{ // setup scene window
		while (!tke::currentWindow) Sleep(100);

		QWindow *window = QWindow::fromWinId((unsigned int)tke::currentWindow->hWnd);
		window->setFlags(Qt::FramelessWindowHint);

		QWidget *widget = QWidget::createWindowContainer(window);
		widget->setFixedWidth(1600);
		widget->setFixedHeight(900);

		ui.scrollArea->setWidget(widget);
	}

	{ // setup pipeline editor
		for (int i = 0; i < 5; i++)
		{
			PipelineEditor::stageEdit[i] = new QLineNumberEdit;
			ui.stageTab->addTab(PipelineEditor::stageEdit[i], tke::StageNames[i].c_str());
		}
		PipelineEditor::findWidget = new QFindWidget;
		ui.stageTab->setCornerWidget(PipelineEditor::findWidget->group);

		PipelineEditor::outputText = new QLineNumberEdit;
		PipelineEditor::outputText->setLineWrapMode(QPlainTextEdit::NoWrap);
		ui.bottomTab->addTab(PipelineEditor::outputText, "Output");
		PipelineEditor::compileText = new QTextBrowser;
		ui.bottomTab->addTab(PipelineEditor::compileText, "Compile");

		tke::AttributeTree at("data");
		at.loadXML("pipelines.xml");
		for (auto c : at.children)
		{
			if (c->name == "pipeline")
			{
				auto a = c->firstAttribute("filename");
				auto pipeline = new tke::Pipeline;
				pipeline->setFilename(a.second);
				pipeline->loadXML();
				pipeline->ext = new PipelineEditor::PipelineExt;
				PipelineEditor::pipelines.push_back(pipeline);

				((PipelineEditor::PipelineExt*)pipeline->ext)->item->setText(a.second.c_str());
				ui.pipelineList->addItem(((PipelineEditor::PipelineExt*)pipeline->ext)->item);
			}
		}
	}

	{ // setup renderer editor

	}
}

void WorldEditor::keyPressEvent(QKeyEvent *k)
{
	if (k->key() == Qt::Key_S)
	{
		if (k->modifiers() == Qt::ControlModifier)
			/*on_saveStage_clicked()*/;
	}
	else if (k->key() == Qt::Key_F5)
	{
		/*on_toSpv_clicked()*/;
	}
}

void WorldEditor::on_pipelineList_currentItemChanged(QListWidgetItem *curr, QListWidgetItem *)
{
	for (auto pipeline : PipelineEditor::pipelines)
	{
		if (((PipelineEditor::PipelineExt*)pipeline->ext)->item == curr)
		{
			if (pipeline == PipelineEditor::currentPipeline) return;

			if (PipelineEditor::currentPipeline)
			{
				for (int i = 0; i < 5; i++)
				{
					if (PipelineEditor::currentPipeline->stages[i])
					{

					}
				}
				for (auto s : currentPipeline->stages)
					s->text = s->wrap.edit->toPlainText().toUtf8().data();
			}

			currentPipeline = pipeline;

			preparingData = true;
			ui.pipelineNameEdit->setText(pipeline->name.c_str());
			pipeline->appear();
			preparingData = false;

			on_find();

			return;
		}
	}
}