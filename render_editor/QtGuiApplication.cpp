#include "QtGuiApplication.h"

#include <qfiledialog.h>
#include <qtreewidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgridlayout.h>

#include <fstream>
#include <sstream> 

#include "d:/TK_Engine/src/core/qt.h"

const std::string rendererPath = "D:/TK_Engine/renderer";

bool appearing = false;

QTreeWidget *tree; 

void setAddButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":image/misc/add.png"));
}

void setRemoveButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":image/misc/delete.png"));
}

void setUpButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":image/misc/up.png"));
}

void setDownButton(QToolButton *b)
{
	b->setMaximumWidth(21);
	b->setMaximumHeight(21);
	b->setIcon(QIcon(":image/misc/down.png"));
}

#include "render.h"

std::vector<Renderer*> renderers;
Renderer *currentRenderer = nullptr;

void saveDataXml()
{
	tke::AttributeTree at("data");

	for (auto r : renderers)
	{
		auto node = new tke::AttributeTreeNode("renderer");
		node->attributes.emplace_back(new tke::NormalVariable("filename", &r->filename), std::string());
	}
}

QtGuiApplication::QtGuiApplication(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	tree = ui.treeWidget;

	tke::AttributeTree at("data");
	at.loadXML("data.xml");

	for (auto c : at.children)
	{
		if (c->name == "renderer")
		{
			auto a = c->firstAttribute("filename");
			auto r = new Renderer;
			r->filename = a.second;
			r->loadXML();
			r->listItem = new QListWidgetItem;
			std::string title = r->filename;
			if (title.compare(0, rendererPath.size(), rendererPath) == 0)
				title = title.c_str() + rendererPath.size();
			r->listItem->setText(title.c_str());
			ui.listWidget->addItem(r->listItem);
			renderers.push_back(r);
		}
	}

	appearing = true;
}

void QtGuiApplication::on_addButton_clicked()
{
	auto list = QFileDialog::getOpenFileNames(this, "", rendererPath.c_str());
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto renderer = new Renderer;
		renderer->filename = list[i].toUtf8().data();
		renderer->loadXML();
		renderer->listItem = new QListWidgetItem;
		std::string title = renderer->filename;
		if (title.compare(0, rendererPath.size(), rendererPath) == 0)
			title = title.c_str() + rendererPath.size();
		renderer->listItem->setText(title.c_str());
		ui.listWidget->addItem(renderer->listItem);
		renderers.push_back(renderer);
	}

	saveDataXml();
}

void QtGuiApplication::on_removeButton_clicked() 
{
	if (!currentRenderer) return;

	for (auto it = renderers.begin(); it != renderers.end(); it++)
	{
		if (*it == currentRenderer)
		{
			renderers.erase(it);
			break;
		}
	}

	delete currentRenderer->listItem;
	delete currentRenderer;
	currentRenderer = nullptr;

	saveDataXml();
}

void QtGuiApplication::on_listWidget_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev)
{
	tree->clear();
	for (auto r : renderers)
	{
		if (r->listItem == _curr)
		{
			currentRenderer = r;
			currentRenderer->appear();
			return;
		}
	}
}

void QtGuiApplication::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
	auto str = (std::string*)item->data(0, Qt::UserRole).value<unsigned int>();
	*str = item->text(0).toUtf8().data();
}

void QtGuiApplication::on_saveButton_clicked()
{
	if (currentRenderer)
		currentRenderer->saveXML();
}