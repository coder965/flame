#pragma once

#include "resource_explorer.h"
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

void openGameExplorer();
SceneMonitorWidget *openSceneMonitorWidget(tke::Scene *s);
ModelMonitorWidget *openModelMonitorWidget(tke::Model *m);
void openAttributeWidget();
void openTextureEditor();

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
	glm::vec3 coord;
	int heightMapIndex = 0;
	int blendMapIndex = 0;
	int colorMap0Index = 0;
	int colorMap1Index = 0;
	int colorMap2Index = 0;
	int colorMap3Index = 0;
	float height = 100;
	bool usePhysx = false;

	void load(tke::AttributeTreeNode *n);
	void save(tke::AttributeTreeNode *n);
};

extern TerrainCreationSetting tcs;

struct WaterCreationSetting
{
	glm::vec3 coord;
	float height = 100;

	void load(tke::AttributeTreeNode *n);
	void save(tke::AttributeTreeNode *n);
};

extern WaterCreationSetting wcs;