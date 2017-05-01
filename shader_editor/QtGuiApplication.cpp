#include "QtGuiApplication.h"

#include <assert.h>
#include <sstream>
#include <regex>
#include <experimental/filesystem>

#include "d:/TK_Engine/src/core/utils.h"
#include "d:/TK_Engine/src/core/render.abstract.h"

auto shaderPath = "D:\\TK_Engine\\shader";
std::string glslangValidatorPath = "d:/VulkanSDK/1.0.37.0/glslang/StandAlone/Release/glslangValidator.exe";

#include "edit.hpp"

QTreeWidget *pipelineTree;
QToolButton *explorerButton;
QTabWidget *bottomTabWidget;
MyEdit *outputMyEdit;
QTextBrowser *compileTextBrowser;
QTabWidget *stageTabWidget;

bool qTextDataPreparing = false;

#include "shader.hpp"

namespace Find
{
	QLineEdit *edit;
	QLabel *label;

	int strSize;
	std::vector<int> datas;
	int current;

	void update()
	{
		Find::label->setText(QString("%1/%2").arg(current + 1).arg(datas.size()));
	}

	void gotoIndex(int index)
	{
		if (datas.size() == 0 || index < 0 || index >= datas.size() || current == index) return;

		auto stage = currentTabStage();
		if (!stage) return;

		stage->edit->setFocus();

		auto cursor = stage->edit->textCursor();
		cursor.setPosition(datas[index]);
		cursor.setPosition(datas[index] + strSize, QTextCursor::KeepAnchor);
		stage->edit->setTextCursor(cursor);

		current = index;
		update();
	}
}

void saveDataXml()
{
	tke::AttributeTree at("data");

	for (auto pipeline : pipelines)
	{
		auto node = new tke::AttributeTreeNode("pipeline");
		node->atrributes.emplace_back(new tke::NormalVariable("filename", &pipeline->filename), std::string());
	}

	at.saveXML("data.xml");
}

QtGuiApplication::QtGuiApplication(QWidget *parent) :
	QMainWindow(parent)
{
	ui.setupUi(this);

	{
		std::ifstream check(glslangValidatorPath);
		assert(check.good());
	}

	pipelineTree = ui.pipelineTree;
	explorerButton = ui.explorerStageFileToolButton;
	bottomTabWidget = ui.bottomTabWidget;
	outputMyEdit = new MyEdit;
	outputMyEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	bottomTabWidget->addTab(outputMyEdit, "Output");
	compileTextBrowser = new QTextBrowser;
	bottomTabWidget->addTab(compileTextBrowser, "Compile");
	stageTabWidget = ui.stageTabWidget;
	{
		Find::edit = new QLineEdit;
		connect(Find::edit, &QLineEdit::textChanged, this, &QtGuiApplication::on_find);
		connect(Find::edit, &QLineEdit::returnPressed, this, &QtGuiApplication::on_find_enter);
		Find::label = new QLabel;
		Find::label->setText("0/0");
		auto btnUp = new QToolButton();
		btnUp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		btnUp->setGeometry(0, 0, 21, 21);
		btnUp->setMaximumSize(21, 21);
		btnUp->setIcon(QIcon(":/png/up.png"));
		connect(btnUp, &QToolButton::clicked, this, &QtGuiApplication::on_find_previous);
		auto btnDown = new QToolButton();
		btnDown->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		btnDown->setGeometry(0, 0, 21, 21);
		btnDown->setMaximumSize(21, 21);
		btnDown->setIcon(QIcon(":/png/down.png"));
		connect(btnDown, &QToolButton::clicked, this, &QtGuiApplication::on_find_next);

		auto layout = new QHBoxLayout;
		layout->addWidget(Find::edit);
		layout->addWidget(Find::label);
		layout->addWidget(btnUp);
		layout->addWidget(btnDown);

		auto group = new QGroupBox();
		group->setLayout(layout);
		group->setStyleSheet("QGroupBox{border:0px;padding-top:-10px;padding-bottom:-10px;}");

		stageTabWidget->setCornerWidget(group);
	}

	tke::AttributeTree at("data");
	at.loadXML("data.xml");

	for (auto c : at.children)
	{
		if (c->name == "pipeline")
		{
			for (auto &a : c->atrributes)
			{
				if (a.first->name == "filename")
				{
					auto pipeline = new Pipeline;
					pipeline->load(a.second);
				}
			}
		}
	}
}

void QtGuiApplication::keyPressEvent(QKeyEvent *k)
{
	if (k->key() == Qt::Key_S)
	{
		if (k->modifiers() == Qt::ControlModifier)
			on_saveStageToolButton_clicked();
	}
	else if (k->key() == Qt::Key_F5)
	{
		on_toSpvToolButton_clicked();
	}
}

void QtGuiApplication::on_pipelineTree_currentItemChanged(QTreeWidgetItem *curr, QTreeWidgetItem *)
{
	for (auto pipeline : pipelines)
	{
		if (pipeline->item == curr)
		{
			if (pipeline == currentPipeline) return;

			if (currentPipeline)
			{
				for (auto s : currentPipeline->stages)
					s->text = s->edit->toPlainText().toUtf8().data();
			}

			qTextDataPreparing = true;
			ui.pipelineNameLineEdit->setText(pipeline->name.c_str());
			pipeline->refreshTabs();
			qTextDataPreparing = false;

			currentPipeline = pipeline;

			on_find();

			return;
		}
	}
}

void QtGuiApplication::on_addPipelineToolButton_clicked()
{
	auto list = QFileDialog::getOpenFileNames(this, "", shaderPath);
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto pipeline = new Pipeline;
		pipeline->load(list[i].toUtf8().data());
		pipelines.push_back(pipeline);
	}

	saveDataXml();
}

void QtGuiApplication::on_removePipelineToolButton_clicked()
{
	if (!currentPipeline) return;

	for (auto it = pipelines.begin(); it != pipelines.end(); it++)
	{
		if (*it == currentPipeline)
		{
			pipelines.erase(it);
			break;
		}
	}

	delete currentPipeline->item;

	delete currentPipeline;
	currentPipeline = nullptr;

	saveDataXml();
}


void QtGuiApplication::on_explorerPipelineFileToolButton_clicked()
{
	if (!currentPipeline) return;

	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		ShellExecuteA(NULL, "open", currentPipeline->filename.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::string cmd = "explorer /select," + currentPipeline->filename;
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}

void QtGuiApplication::on_addStageToolButton_clicked()
{
	if (!currentPipeline || currentPipeline->stages.size() >= 5) return;

	bool ok;
	QString text = QInputDialog::getText(this, "File Name",
		"(Create when not exist)",
		QLineEdit::Normal, "", &ok);

	if (!ok) return;

	std::string filename = text.toUtf8().data();
	std::experimental::filesystem::path p(filename);
	auto type = tke::StageFlagByExt(p.extension().string());

	for (auto s : currentPipeline->stages)
	{
		if (s->type == type)
			return;
	}

	auto s = new Stage;
	s->type = type;
	s->filename = filename;
	s->filepath = p.parent_path().string();

	tke::OnceFileBuffer file(currentPipeline->filepath + "/" + s->filename);
	s->text = file.data;

	currentPipeline->stages.push_back(s);

	qTextDataPreparing = true;
	currentPipeline->refreshTabs();
	qTextDataPreparing = false;

	currentPipeline->saveXML();
}

void QtGuiApplication::on_removeStageToolButton_clicked()
{
	auto s = currentTabStage();
	if (!s) return;

	currentPipeline->removeStage(s);

	currentPipeline->saveXML();
}

void QtGuiApplication::on_saveStageToolButton_clicked()
{
	auto s = currentTabStage();
	if (!s || !s->changed) return;

	s->text = s->edit->toPlainText().toUtf8().data();

	std::ofstream file(currentPipeline->filepath + "/" + s->filename);
	file << s->text;

	s->changed = false;

	stageTabWidget->setTabText(s->tabIndex, stageNames[(int)s->type].c_str());
}

void QtGuiApplication::on_toSpvToolButton_clicked()
{
	auto s = currentTabStage();
	if (!s) return;

	on_saveStageToolButton_clicked();

	outputMyEdit->clear();
	compileTextBrowser->clear();

	auto string = s->getFullText(currentPipeline->filepath);
	outputMyEdit->setPlainText(string.c_str());
	s->output = string;

	std::ofstream file("temp.glsl");
	file << string;
	file.close();

	std::string spvFilename = currentPipeline->filepath + "/" + s->filename + ".spv";
	std::string cmd = "/C " + glslangValidatorPath + "-V temp.glsl -S " + stageNames[(int)s->type] + " -q -o " + spvFilename + " >> output.txt";
	tke::exec("cmd", cmd);
	tke::OnceFileBuffer output("output.txt");
	s->compileOutput = output.data;

	compileTextBrowser->setText(output.data);

	std::stringstream outputStream(s->compileOutput);

	std::string line;
	while (!outputStream.eof())
	{
		std::getline(outputStream, line);

		std::regex pat(R"(ERROR:.*)");
		std::smatch sm;
		if (std::regex_search(line, sm, pat))
		{
			QMessageBox::information(this, "Oh Shit", "Shader Compile Failed", QMessageBox::Ok);
			break;
		}
	}

	DeleteFileA("output.txt");
	DeleteFileA("temp.glsl");

	auto pipeName = R"(\\.\pipe\tke)";

	if (WaitNamedPipeA(pipeName, NMPWAIT_WAIT_FOREVER))
	{
		auto hPipe = CreateFileA(pipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD length = 0;
		std::string cmd = "reload " + currentPipeline->name;
		WriteFile(hPipe, cmd.data(), cmd.size() + 1, &length, NULL);
		CloseHandle(hPipe);
	}

}

void QtGuiApplication::on_stageTabWidget_tabBarClicked(int index)
{
	currentPipeline->setTabData(index);
}

void QtGuiApplication::on_explorerStageFileToolButton_clicked()
{
	auto s = currentTabStage();
	if (!s) return;
	
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		std::string cmd = currentPipeline->filepath + "/" + s->filename;
		ShellExecuteA(NULL, "open", cmd.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::string cmd = "explorer /select," + currentPipeline->filepath + "/" + s->filename;
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}

void QtGuiApplication::on_savePipelineToolButton_clicked()
{
	if (qTextDataPreparing || !currentPipeline) return;

	currentPipeline->name = ui.pipelineNameLineEdit->text().toUtf8().data();

	saveDataXml();
}

void QtGuiApplication::on_find()
{
	auto s = currentTabStage();
	if (!s) return;

	QList<QTextEdit::ExtraSelection> extraSelections;

	Find::datas.clear();
	Find::current = -1;

	auto findStr = Find::edit->text();
	Find::strSize = findStr.size();

	auto str = s->edit->toPlainText();

	if (Find::strSize > 0)
	{
		auto offset = 0;
		while (true)
		{
			offset = str.indexOf(findStr, offset + Find::strSize);
			if (offset == -1) break;

			Find::datas.push_back(offset);

			QTextEdit::ExtraSelection selection;

			QColor lineColor = QColor(Qt::yellow).lighter(160);

			selection.format.setBackground(lineColor);
			selection.cursor = s->edit->textCursor();
			selection.cursor.setPosition(offset);
			selection.cursor.setPosition(offset + Find::strSize, QTextCursor::KeepAnchor);
			extraSelections.append(selection);
		}
	}

	s->edit->setExtraSelections(extraSelections);

	Find::update();
}

void QtGuiApplication::on_find_enter()
{
	Find::gotoIndex(0);
}

void QtGuiApplication::on_find_previous()
{
	Find::gotoIndex(Find::current - 1);
}

void QtGuiApplication::on_find_next()
{
	Find::gotoIndex(Find::current + 1);
}

void QtGuiApplication::on_stageTabWidget_currentChanged(int index)
{
	on_find();
}

