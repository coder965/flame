#include "QtGuiApplication.h"

#include <assert.h>
#include <sstream>
#include <regex>
#include <experimental/filesystem>

#include "d:/TK_Engine/src/core/utils.h"
#include "d:/TK_Engine/src/core/render.abstract.h"

const std::string shaderPath("D:/TK_Engine/pipeline");

#include "edit.hpp"

QListWidget *pipelineList;
QToolButton *explorerButton;
QTabWidget *bottomTab;
MyEdit *outputText;
QTextBrowser *compileText;
QTreeWidget *attributeTree;
QTabWidget *stageTab;

bool preparingData = false;

#include "pipeline.h"

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

		stage->wrap.edit->setFocus();

		auto cursor = stage->wrap.edit->textCursor();
		cursor.setPosition(datas[index]);
		cursor.setPosition(datas[index] + strSize, QTextCursor::KeepAnchor);
		stage->wrap.edit->setTextCursor(cursor);

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
		node->attributes.emplace_back(new tke::NormalVariable("filename", &pipeline->filename), std::string());
	}

	at.saveXML("data.xml");
}

#include "def.h"
extern "C" {
	extern FILE *yyin;
	extern int yylex();
	extern int yylineno;
	extern char *yytext;
}

QtGuiApplication::QtGuiApplication(QWidget *parent) :
	QMainWindow(parent)
{
	ui.setupUi(this);

	pipelineList = ui.pipelineList;
	explorerButton = ui.explorerStage;
	bottomTab = ui.bottomTab;
	outputText = new MyEdit;
	outputText->setLineWrapMode(QPlainTextEdit::NoWrap);
	bottomTab->addTab(outputText, "Output");
	compileText = new QTextBrowser;
	bottomTab->addTab(compileText, "Compile");
	attributeTree = ui.attributeTree;
	stageTab = ui.stageTab;
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
		btnUp->setIcon(QIcon(":/image/misc/up.png"));
		connect(btnUp, &QToolButton::clicked, this, &QtGuiApplication::on_find_previous);
		auto btnDown = new QToolButton();
		btnDown->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		btnDown->setGeometry(0, 0, 21, 21);
		btnDown->setMaximumSize(21, 21);
		btnDown->setIcon(QIcon(":/image/misc/down.png"));
		connect(btnDown, &QToolButton::clicked, this, &QtGuiApplication::on_find_next);

		auto layout = new QHBoxLayout;
		layout->addWidget(Find::edit);
		layout->addWidget(Find::label);
		layout->addWidget(btnUp);
		layout->addWidget(btnDown);

		auto group = new QGroupBox();
		group->setLayout(layout);
		group->setStyleSheet("QGroupBox{border:0px;padding-top:-10px;padding-bottom:-10px;}");

		stageTab->setCornerWidget(group);
	}

	tke::AttributeTree at("data");
	at.loadXML("data.xml");

	for (auto c : at.children)
	{
		if (c->name == "pipeline")
		{
			auto a = c->firstAttribute("filename");
			auto pipeline = new Pipeline;
			pipeline->load(a.second);
			pipelines.push_back(pipeline);
		}
	}
}

void QtGuiApplication::keyPressEvent(QKeyEvent *k)
{
	if (k->key() == Qt::Key_S)
	{
		if (k->modifiers() == Qt::ControlModifier)
			on_saveStage_clicked();
	}
	else if (k->key() == Qt::Key_F5)
	{
		on_toSpv_clicked();
	}
}

void QtGuiApplication::on_pipelineList_currentItemChanged(QListWidgetItem *curr, QListWidgetItem *)
{
	for (auto pipeline : pipelines)
	{
		if (pipeline->item == curr)
		{
			if (pipeline == currentPipeline) return;

			if (currentPipeline)
			{
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

void QtGuiApplication::on_addPipeline_clicked()
{
	auto list = QFileDialog::getOpenFileNames(this, "", shaderPath.c_str());
	if (list.size() == 0) return;

	for (auto i = 0; i < list.size(); i++)
	{
		auto pipeline = new Pipeline;
		pipeline->load(list[i].toUtf8().data());
		pipelines.push_back(pipeline);
	}

	saveDataXml();
}

void QtGuiApplication::on_removePipeline_clicked()
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


void QtGuiApplication::on_explorerPipeline_clicked()
{
	if (!currentPipeline) return;

	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		ShellExecuteA(NULL, "open", currentPipeline->filename.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::string cmd("explorer /select," + currentPipeline->filename);
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}

void QtGuiApplication::on_addStage_clicked()
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

	preparingData = true;
	currentPipeline->appear();
	preparingData = false;

	currentPipeline->saveXML();
}

void QtGuiApplication::on_removeStage_clicked()
{
	auto s = currentTabStage();
	if (!s) return;

	currentPipeline->removeStage(s);
	currentPipeline->saveXML();
}

void QtGuiApplication::on_saveStage_clicked()
{
	auto s = currentTabStage();
	if (!s || !s->wrap.changed) return;

	s->text = s->wrap.edit->toPlainText().toUtf8().data();

	std::ofstream file(currentPipeline->filepath + "/" + s->filename);
	file << s->text;

	s->wrap.changed = false;

	stageTab->setTabText(s->wrap.tabIndex, stageNames[(int)s->type].c_str());
}

void QtGuiApplication::on_toSpv_clicked()
{
	auto s = currentTabStage();
	if (!s) return;

	on_saveStage_clicked();

	outputText->clear();
	compileText->clear();

	auto string = s->getFullText(currentPipeline->filepath);
	outputText->setPlainText(string.c_str());
	s->output = string;

	std::ofstream file("temp.glsl");
	file.write(string.c_str(), string.size());
	file.close();

	std::experimental::filesystem::path spv(currentPipeline->filepath + "/" + s->filename + ".spv");

	tke::exec("cmd", "/C glslangValidator -V temp.glsl -S " + stageNames[(int)log2((int)s->type)] + " -q -o " + spv.string() + " >> output.txt");
	tke::OnceFileBuffer output("output.txt");
	s->compileOutput = output.data;
	compileText->setText(output.data);

	yyin = fopen("output.txt", "rb");

	int token = yylex();
	while (token)
	{
		if (token == ERROR_MK)
		{
			QMessageBox::information(this, "Oh Shit", "Shader Compile Failed", QMessageBox::Ok);
			break;
		}
		token = yylex();
	}
	fclose(yyin);
	yyin = NULL;

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

void QtGuiApplication::on_stageTab_currentChanged(int index)
{
	if (preparingData) return;

	currentPipeline->setTabData(index);
	on_find();
}

void QtGuiApplication::on_explorerStage_clicked()
{
	auto s = currentTabStage();
	if (!s) return;
	
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		std::string cmd(currentPipeline->filepath + "/" + s->filename);
		ShellExecuteA(NULL, "open", cmd.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::experimental::filesystem::path p(currentPipeline->filepath + "/" + s->filename);
		std::string cmd("explorer /select," + std::experimental::filesystem::absolute(p).string());
		WinExec(cmd.c_str(), SW_SHOWNORMAL);
	}
}

void QtGuiApplication::on_savePipeline_clicked()
{
	if (preparingData || !currentPipeline) return;

	currentPipeline->name = ui.pipelineNameEdit->text().toUtf8().data();

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

	auto str = s->wrap.edit->toPlainText();

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
			selection.cursor = s->wrap.edit->textCursor();
			selection.cursor.setPosition(offset);
			selection.cursor.setPosition(offset + Find::strSize, QTextCursor::KeepAnchor);
			extraSelections.append(selection);
		}
	}

	s->wrap.edit->setExtraSelections(extraSelections);

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
