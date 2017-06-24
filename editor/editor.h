#pragma once

#include "../src/scene.h"

extern tke::Image *titleImage;

struct EditorWindow : tke::Window
{
	EditorWindow();
	virtual ~EditorWindow() override;
	void openGameExplorer();
	void openOutputWidget();
	void openMonitorWidget(const std::string &renderer_filename, tke::Model *m);
	void openAttributeWidget();
	void openBoneMotionWidget();
	virtual void renderEvent() override;
	void saveUi(const std::string &filename);
	void loadUi(const std::string &filename);
};

extern EditorWindow *mainWindow;