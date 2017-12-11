#include "../../../src/ui/ui.h"

#include "terrain_editor.h"

std::string TerrainEditorClass::getName()
{
	return "terrain editor";
}

Window *TerrainEditorClass::load(tke::AttributeTreeNode *n)
{
	auto w = new TerrainEditor;
	return w;
}

TerrainEditorClass terrainEditorClass;

TerrainEditor::TerrainEditor()
	:Window(&terrainEditorClass), layer(true)
{
	auto vxcount = block_count + 1;
	vertexs.resize(vxcount * vxcount);
	glm::vec3 center = glm::vec3(block_count * -0.5f, 0.f, block_count * -0.5f);
	for (int y = 0; y < vxcount; y++)
	{
		for (int x = 0; x < vxcount; x++)
		{
			vertexs[y * vxcount + x] = {
				glm::vec3(x, 0, y) + center,
				glm::vec2(0),
				glm::vec3(0, 1, 0),
				glm::vec3(0)
			};
		}
	}

	indices.resize(6 * block_count * block_count);
	for (int y = 0; y < block_count; y++)
	{
		for (int x = 0; x < block_count; x++)
		{
			auto idx = (y * block_count + x) * 6;
			indices[idx + 0] = y * vxcount + x;
			indices[idx + 1] = (y + 1) * vxcount + x;
			indices[idx + 2] = y * vxcount + x + 1;
			indices[idx + 3] = y * vxcount + x + 1;
			indices[idx + 4] = (y + 1) * vxcount + x;
			indices[idx + 5] = (y + 1) * vxcount + x + 1;
		}
	}

	vertex_buffer = std::make_unique<tke::VertexBuffer>(sizeof(tke::VertexStat) * vertexs.size(), vertexs.data());
	index_buffer = std::make_unique<tke::IndexBuffer>(sizeof(int) * indices.size(), indices.data());

	camera.setMode(tke::CameraMode::targeting);
	renderer = std::make_unique<tke::PlainRenderer>();
}

void TerrainEditor::show()
{
	ImGui::Begin("Terrain -", &opened);

	ImVec2 image_pos = ImGui::GetCursorScreenPos();
	ImVec2 image_size = ImVec2(layer.image->levels[0].cx, layer.image->levels[0].cy);
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(layer.image->index), image_pos, image_pos + image_size);
	if (ImGui::IsItemHovered())
	{
		if (tke::mouseDispX != 0 || tke::mouseDispY != 0)
		{
			auto distX = (float)tke::mouseDispX / (float)tke::resCx;
			auto distY = (float)tke::mouseDispY / (float)tke::resCy;
			if (tke::mouseMiddle.pressing)
				camera.rotateByCursor(distX, distY);
		}
		if (tke::mouseScroll != 0)
			camera.scroll(tke::mouseScroll);
	}
	if (camera.changed)
		camera.lookAtTarget();

	ImGui::End();

	auto cb_list = tke::addFrameCommandBufferList();
	{
		tke::PlainRenderer::DrawData data;
		data.mode = tke::PlainRenderer::mode_color_and_front_light;
		data.vbuffer0 = vertex_buffer.get();
		data.ibuffer = index_buffer.get();
		tke::PlainRenderer::DrawData::ObjData obj_data;
		obj_data.mat = glm::mat4(1);
		obj_data.color = glm::vec4(1.f);
		tke::PlainRenderer::DrawData::ObjData::GeoData geo_data;
		geo_data.index_count = indices.size();
		obj_data.geo_data.push_back(geo_data);
		data.obj_data.push_back(obj_data);
		renderer->render(cb_list, layer.framebuffer.get(), true, &camera, &data);
	}
}
