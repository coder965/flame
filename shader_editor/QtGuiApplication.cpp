#include "QtGuiApplication.h"

#include <sstream>
#include <experimental/filesystem>

#include "d:/TK_Engine/src/core/utils.h"
#include "d:/TK_Engine/src/core/render.abstract.h"

auto shaderPath = "D:\\TK_Engine\\shader";
auto glslangValidatorPath = "d:/VulkanSDK/1.0.37.0/glslang/StandAlone/Release/glslangValidator.exe";

#include "edit.hpp"

QTreeWidget *pipelineTree;
QToolButton *explorerButton;
QTabWidget *bottomTabWidget;
MyEdit *outputMyEdit;
QTextBrowser *compileTextBrowser;
QTabWidget *stageTabWidget;

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

		auto stage = getCurrentStage();
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

bool qTextDataPreparing = false;

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

	connect(qTexts[0], &QPlainTextEdit::textChanged, this, &QtGuiApplication::on_text0_changed);
	connect(qTexts[1], &QPlainTextEdit::textChanged, this, &QtGuiApplication::on_text1_changed);
	connect(qTexts[2], &QPlainTextEdit::textChanged, this, &QtGuiApplication::on_text2_changed);
	connect(qTexts[3], &QPlainTextEdit::textChanged, this, &QtGuiApplication::on_text3_changed);
	connect(qTexts[4], &QPlainTextEdit::textChanged, this, &QtGuiApplication::on_text4_changed);
}

void on_text_changed(int type)
{
	if (qTextDataPreparing) return;

	for (auto &s : currentPipeline->stages)
	{
		if ((int)s.type == type)
		{
			if (!s.changed)
			{
				s.changed = true;
				stageTabWidget->setTabText(qTabIndexs[type], QString(stageNames[type]) + "*");
			}
		}
	}

}

void QtGuiApplication::on_text0_changed()
{
	on_text_changed(0);
}

void QtGuiApplication::on_text1_changed()
{
	on_text_changed(1);
}

void QtGuiApplication::on_text2_changed()
{
	on_text_changed(2);
}

void QtGuiApplication::on_text3_changed()
{
	on_text_changed(3);
}

void QtGuiApplication::on_text4_changed()
{
	on_text_changed(4);
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

void QtGuiApplication::on_pipelineTree_currentItemChanged(QTreeWidgetItem *_curr, QTreeWidgetItem *_prev)
{
	for (auto pipeline : pipelines)
	{
		if (pipeline->item == _curr)
		{
			if (pipeline == currentPipeline) return;

			if (currentPipeline)
			{
				for (int i = 0; i < 5; i++)
				{
					if (currentPipeline->stages[i])
						currentPipeline->qTextToStageStr(i);
				}
			}

			qTextDataPreparing = true;
			ui.pipelineNameLineEdit->setText(pipeline->name);
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

	char cmd[260];
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		sprintf(cmd, "%s", currentPipeline->filename);
		ShellExecuteA(NULL, "open", cmd, nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		sprintf(cmd, "explorer /select,%s", currentPipeline->filename);
		WinExec(cmd, SW_SHOWNORMAL);
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
	char ext[32];
	tk::getFileExt(filename, ext);

	auto index = getStageIndex(ext);
	if (index == -1) return;

	if (currentPipeline->stages[index]) return;

	auto stage = new Stage;
	strcpy(stage->filename, filename);
	tk::getFilePath(stage->filename, stage->filepath);
	stage->type = index;

	sprintf(filename, "%s%s", currentPipeline->filepath, stage->filename);
	auto f = fopen(filename, "rb");
	if (f)
	{
		tk::loadFile(f, &stage->str);
		fclose(f);
	}
	else
	{
		f = fopen(filename, "wb");
		fclose(f);
	}

	currentPipeline->stages[index] = stage;
	qTextDataPreparing = true;
	currentPipeline->refreshTabs();
	qTextDataPreparing = false;

	currentPipeline->saveXml();
}

void QtGuiApplication::on_removeStageToolButton_clicked()
{
	auto index = stageTabWidget->currentIndex();
	if (index == -1) return;

	stageTabWidget->removeTab(index);

	for (int i = 0; i < 5; i++)
	{
		if (qTabIndexs[i] == index)
		{
			delete currentPipeline->stages[i];
			currentPipeline->stages[i] = nullptr;
			currentPipeline->saveXml();
			return;
		}
	}
}

void QtGuiApplication::on_saveStageToolButton_clicked()
{
	auto i = getCurrentStageIndex();
	if (i == -1) return;

	auto stage = currentPipeline->stages[i];
	if (!stage->changed) return;

	currentPipeline->qTextToStageStr(i);

	char filename[260];
	sprintf(filename, "%s%s", currentPipeline->filepath, stage->filename);

	auto f = fopen(filename, "wb");
	fprintf(f, "%s", stage->str);
	fclose(f);

	stage->changed = false;
	stageTabWidget->setTabText(stageTabWidget->currentIndex(), stageNames[stage->type]);
}

void QtGuiApplication::on_toSpvToolButton_clicked()
{
	on_saveStageToolButton_clicked();

	outputMyEdit->clear();
	compileTextBrowser->clear();

	auto i = getCurrentStageIndex();
	if (i == -1) return;

	auto stage = currentPipeline->stages[i];

	auto string = stage->getFullText();
	outputMyEdit->setPlainText(string);
	tk::setStr(&stage->output, string.toUtf8().data());

	auto f = fopen("temp.glsl", "wb");
	fprintf(f, "%s", string.toUtf8().data());
	fclose(f);

	char spvFilename[260];
	char cmd[260];
	sprintf(spvFilename, "%s%s.spv", currentPipeline->filepath, stage->filename);
	sprintf(cmd, "/C %s -V temp.glsl -S %s -q -o %s >> output.txt", glslangValidatorPath, stageNames[stage->type], spvFilename);
	tk::Win::exec("cmd", cmd);
	char *output = nullptr;
	tk::loadFile("output.txt", &output);
	if (output)
	{
		tk::setStr(&stage->compileOutput, output);

		auto outputString = QString(output);
		compileTextBrowser->setText(outputString);

		auto list = outputString.split("\n");
		for (int i = 0; i < list.size(); i++)
		{
			auto line = list[i];

			QString pat = R"(ERROR:.*)";
			QRegExp reg(pat);
			auto firstPos = reg.indexIn(line);
			if (firstPos >= 0)
			{
				QMessageBox::information(this, "Oh Shit", "Shader Compile Failed", QMessageBox::Ok);
				break;
			}
		}

		delete[]output;
		DeleteFileA("output.txt");
	}

	DeleteFileA("temp.glsl");

	auto pipeName = R"(\\.\pipe\tke)";

	if (WaitNamedPipeA(pipeName, NMPWAIT_WAIT_FOREVER))
	{
		auto hPipe = CreateFileA(pipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD length = 0;
		char buf[260];
		sprintf(buf, "reload %s", currentPipeline->name);
		WriteFile(hPipe, buf, strlen(buf) + 1, &length, NULL);
		CloseHandle(hPipe);
	}

}

void QtGuiApplication::on_stageTabWidget_tabBarClicked(int index)
{
	currentPipeline->setTabData(index);
}

void QtGuiApplication::on_explorerStageFileToolButton_clicked()
{
	auto index = getCurrentStageIndex();
	if (index == -1) return;

	char cmd[260];
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		sprintf(cmd, "%s%s", currentPipeline->filepath, currentPipeline->stages[index]->filename);
		ShellExecuteA(NULL, "open", cmd, nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::string cmd = "explorer /select," + currentPipeline->filepath + "/" + currentPipeline->stages[index]->filename;
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}

void QtGuiApplication::on_savePipelineToolButton_clicked()
{
	if (qTextDataPreparing || !currentPipeline) return;

	auto str = ui.pipelineNameLineEdit->text();
	if (str.size() < 50) strcpy(currentPipeline->name, str.toUtf8().data());

	saveDataXml();
}

void QtGuiApplication::on_find()
{
	auto index = getCurrentStageIndex();
	if (index == -1) return;

	auto text = qTexts[index];

	QList<QTextEdit::ExtraSelection> extraSelections;

	Find::datas.clear();
	Find::current = -1;

	auto findStr = Find::edit->text();
	Find::strSize = findStr.size();

	auto str = text->toPlainText();

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
			selection.cursor = text->textCursor();
			selection.cursor.setPosition(offset);
			selection.cursor.setPosition(offset + Find::strSize, QTextCursor::KeepAnchor);
			extraSelections.append(selection);
		}
	}

	text->setExtraSelections(extraSelections);

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

