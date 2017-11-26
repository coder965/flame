#pragma once

#include <filesystem>

#include "window/resource_explorer.h"
#include "window/scene_editor.h"

extern std::experimental::filesystem::path project_path;

struct ObjectCreationSetting
{
	int modelIndex = 0;
	bool use_camera_position = false;
	bool use_camera_target_position = false;
	glm::vec3 coord = glm::vec3(0.f);
	glm::vec3 euler = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
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