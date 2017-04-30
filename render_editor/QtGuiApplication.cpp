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

#define TKE_DRAWCALL_EXT_TYPE DrawcallExtraData
//#define TKE_PUSHCONSTANT_EXT_TYPE PushConstantExtraData
#define TKE_DEPENDENCY_EXT_TYPE DependencyExtraData
#define TKE_DRAWACTION_EXT_TYPE DrawActionExtraData
#define TKE_ATTACHMENT_EXT_TYPE AttachmentExtraData
#define TKE_RENDERPASS_EXT_TYPE RenderPassExtraData
#define TKE_RENDERER_EXT_TYPE RendererExtraData
#include "d:/tk-sdk/utils.h"
#include "d:/tk-sdk/qt.h"
#include "d:/tk-sdk/engine/core/render.h"

#include "d:/rapidxml-1.13/rapidxml.hpp"
#include "d:/rapidxml-1.13/rapidxml_utils.hpp"
#include "d:/rapidxml-1.13/rapidxml_print.hpp"

auto rendererPath = "D:/tk-sdk/engine/renderer";

bool appearing = false;

QTreeWidget *tree;

#include "render.h"

std::vector<tk::Engine::Renderer*> renderers;
tk::Engine::Renderer *currentRenderer = nullptr;

void saveDataXml()
{
	rapidxml::xml_document<> xmlDoc;
	auto dataNode = xmlDoc.allocate_node(rapidxml::node_element, "data");
	xmlDoc.append_node(dataNode);

	for (auto r : renderers)
	{
		auto node = xmlDoc.allocate_node(rapidxml::node_element, "renderer");
		node->append_attribute(xmlDoc.allocate_attribute("name", r->name.c_str()));
		node->append_attribute(xmlDoc.allocate_attribute("filename", r->filename.c_str()));
		dataNode->append_node(node);
	}

	std::string dst;
	rapidxml::print(std::back_inserter(dst), xmlDoc);

	auto f = fopen("data.xml", "wb");
	fprintf(f, dst.c_str());
	fclose(f);

	xmlDoc.clear();
}

QtGuiApplication::QtGuiApplication(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	tree = ui.treeWidget;

	char *dataFile;
	tk::loadFile("data.xml", &dataFile);

	rapidxml::xml_document<> dataXmlDoc;
	dataXmlDoc.parse<0>(dataFile);

	auto firstNode = dataXmlDoc.first_node();
	if (strcmp(firstNode->name(), "data") == 0)
	{
		for (auto n = firstNode->first_node(); n; n = n->next_sibling())
		{
			if (strcmp(n->name(), "renderer") == 0)
			{
				auto renderer = new tk::Engine::Renderer;
				auto a = n->first_attribute("name");
				if (a) renderer->name = a->value();
				a = n->first_attribute("filename");
				if (a)
				{
					renderer->filename = a->value();
					renderer->loadXML();
					setupRenderer(renderer);
					renderer->ext->listItem = new QListWidgetItem;
					std::string title = renderer->filename;
					if (title.compare(0, strlen(rendererPath), rendererPath) == 0)
						title = title.c_str() + strlen(rendererPath);
					renderer->ext->listItem->setText(title.c_str());
					ui.listWidget->addItem(renderer->ext->listItem);
				}

				renderers.push_back(renderer);
			}
		}
	}
	delete[]dataFile;

	appearing = true;
}

void QtGuiApplication::on_addButton_clicked()
{
	auto list = QFileDialog::getOpenFileNames(this, "", rendererPath);
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto renderer = new tk::Engine::Renderer;
		renderer->filename = list[i].toUtf8().data();
		renderer->loadXML();
		setupRenderer(renderer);
		renderer->ext->listItem = new QListWidgetItem;
		std::string title = renderer->filename;
		if (title.compare(0, strlen(rendererPath), rendererPath) == 0)
			title = title.c_str() + strlen(rendererPath);
		renderer->ext->listItem->setText(title.c_str());
		ui.listWidget->addItem(renderer->ext->listItem);
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

	currentRenderer->ext->cleanUp();
	delete currentRenderer->ext->listItem;
	delete currentRenderer;
	currentRenderer = nullptr;

	saveDataXml();
}

void QtGuiApplication::on_listWidget_currentItemChanged(QListWidgetItem *_curr, QListWidgetItem *_prev)
{
	if (currentRenderer)
	{
		currentRenderer->ext->passesItem.item = nullptr;
		tree->clear();
	}
	for (auto r : renderers)
	{
		if (r->ext->listItem == _curr)
		{
			currentRenderer = r;
			currentRenderer->ext->appear();
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