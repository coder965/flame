#pragma once

#include "../src/entity.h"

#include "game.h"
#include "monitor.h"

enum LastWindowType
{
	LastWindowTypeNull,
	LastWindowTypeGameExplorer,
	LastWindowTypeMonitor
};
extern LastWindowType lastWindowType;
extern MonitorWidget *lastMonitorWidget;

extern tke::Image *titleImage;

struct EditorWindow : tke::Window
{
	EditorWindow();
	virtual ~EditorWindow() override;
	void openGameExplorer();
	void openMonitorWidget(tke::Scene *s);
	void openAttributeWidget();
	virtual void renderEvent() override;
};

extern EditorWindow *mainWindow;

struct ObjectCreationSetting
{
	int modelIndex = 0;
	bool use_camera_position = false;
	bool use_camera_target_position = false;
	glm::vec3 coord;
	bool randC[3] = {};
	float coordRandRange = 1.f;
	glm::vec3 euler;
	bool randR[3] = {};
	float eulerRandRange = 360.f;
	glm::vec3 scale = glm::vec3(1.f);
	bool randS[3] = {};
	float scaleRandRange = 1.f;
	bool same_scale_rand = false;
	int physxType = 0;

	void load(tke::AttributeTreeNode *n);
	void save(tke::AttributeTreeNode *n);
};

extern ObjectCreationSetting ocs;

struct TerrainCreationSetting
{
	int heightMapIndex = 0;
	int colorMapIndex = 0;
	float height = 100;
	bool usePhysx = false;

	void load(tke::AttributeTreeNode *n);
	void save(tke::AttributeTreeNode *n);
};

extern TerrainCreationSetting tcs;