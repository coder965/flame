#include "QtGuiApplication.h"

#include "def.h"
extern "C" {
	extern FILE *yyin;
	extern int yylex();
	int yylex_destroy();
	extern char *yytext;
}

bool preparingData = false;

#include "pipeline.h"

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
		std::experimental::filesystem::path p(currentPipeline->filename);
		std::string cmd("explorer /select," + std::experimental::filesystem::absolute(p).string());
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

	currentPipeline->changed = true;
	currentPipeline->setTitle();
}

void QtGuiApplication::on_removeStage_clicked()
{
	auto s = currentTabStage();
	if (!s) return;

	currentPipeline->removeStage(s);

	currentPipeline->changed = true;
	currentPipeline->setTitle();
}

void QtGuiApplication::on_saveStage_clicked()
{
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		if (!currentPipeline) return;

		for (auto s : currentPipeline->stages)
			s->on_save();
	}
	else
	{
		auto s = currentTabStage();
		if (!s || !s->wrap.changed) return;

		s->on_save();
	}
}

void QtGuiApplication::on_toSpv_clicked()
{
	Stage *s = nullptr;
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		if (!currentPipeline) return;

		for (auto s : currentPipeline->stages)
			s->on_spv();

		s = currentTabStage();
		if (!s) return;
	}
	else
	{
		s = currentTabStage();
		if (!s) return;

		s->on_spv();
	}
	outputText->setPlainText(s->output.c_str());
	compileText->setText(s->compileOutput.c_str());

	currentPipeline->changed = true;
	currentPipeline->setTitle();
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
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		for (auto p : pipelines)
			p->on_save();
	}
	else
	{
		if (!currentPipeline) return;
		currentPipeline->on_save();
	}
}

