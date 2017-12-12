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

		if (tke::mouseLeft.justDown || tke::mouseRight.justDown)
		{
			if (!tke::keyStates[VK_SHIFT].pressing && !tke::keyStates[VK_CONTROL].pressing)
			{
				auto winx = tke::mouseX - image_pos.x;
				auto winy = tke::mouseY - image_pos.y;

				auto p0 = glm::unProject(glm::vec3(winx, winy, -1.f), camera.getMatInv(), tke::matPerspective, glm::vec4(0, 0, tke::resCx, tke::resCy));
				auto p1 = glm::unProject(glm::vec3(winx, winy, 1.f), camera.getMatInv(), tke::matPerspective, glm::vec4(0, 0, tke::resCx, tke::resCy));
				auto t = p0.y / (p0.y - p1.y);
				if (t > 0 && t < 1)
				{
					auto hs = block_count * 0.5f;
					auto lx = p0.x + t * (p1.x - p0.x) + hs;
					auto lz = p0.z + t * (p1.z - p0.z) + hs;
					if (lx > 0 && lx < block_count && lz > 0 && lz < block_count)
					{
						auto vxcount = block_count + 1;
						auto fl_x = glm::floor(lx);
						auto fl_y = glm::floor(lz);

						auto fChangeHeight = [&](float x, float y, float px, float py) {
							auto dis = glm::distance(glm::vec2(x, y), glm::vec2(px, py));
							vertexs[y * vxcount + x].position.y += dis * 
								(tke::mouseLeft.justDown ? 1.f : -1.f);
							//vertexs[y * vxcount + x].normal = glm::vec3(0.f);
						};

						fChangeHeight(fl_x, fl_y, lx, lz);
						fChangeHeight(fl_x + 1, fl_y, lx, lz);
						fChangeHeight(fl_x + 1, fl_y + 1, lx, lz);
						fChangeHeight(fl_x, fl_y + 1, lx, lz);

						auto fGetHeight = [&](float x, float y) {
							x = glm::clamp(x, 0.f, (float)block_count);
							y = glm::clamp(y, 0.f, (float)block_count);
							return vertexs[y * vxcount + x].position.y;
						};

						auto fCalcNormal = [&](float x, float y) {
							if (x < 0 || x > block_count || y < 0 || y > block_count)
								return;

							float hC = fGetHeight(x, y);
							float hL = fGetHeight(x - 1, y);
							float hR = fGetHeight(x + 1, y);
							float hU = fGetHeight(x, y - 1);
							float hD = fGetHeight(x, y + 1);

							auto v0 = glm::vec3(0.5f, (hC + hR) * 0.5f, 0.f) - glm::vec3(-0.5, (hC + hL) * 0.5, 0.f);
							auto v1 = glm::vec3(0.f, (hC + hD) * 0.5f, 0.5f) - glm::vec3(0.f, (hC + hU) * 0.5, -0.5f);

							vertexs[y * vxcount + x].normal = glm::normalize(-glm::cross(v0, v1));
						};

						for (auto y = fl_y - 1; y <= fl_y + 2; y++)
						{
							for (auto x = fl_x - 1; x <= fl_x + 2; x++)
								fCalcNormal(x, y);
						}

						auto left = glm::max(0.f, fl_x - 1);
						auto right = glm::min((float)block_count, fl_x + 2);
						auto top = glm::max(0.f, fl_y - 1);
						auto bottom = glm::min((float)block_count, fl_y + 2);
						auto xlength = (int)(right - left);
						auto ylength = (int)(bottom - top);
						tke::StagingBuffer stagingBuffer(xlength * ylength * sizeof(tke::VertexStat));
						auto map = (tke::VertexStat*)stagingBuffer.map(0, stagingBuffer.size);
						std::vector<VkBufferCopy> ranges(ylength);
						for (int i = 0; i < ranges.size(); i++)
						{
							memcpy(map + i * xlength, &vertexs[(top + i) * vxcount + left], xlength * sizeof(tke::VertexStat));
							ranges[i].size = xlength * sizeof(tke::VertexStat);
							ranges[i].dstOffset = ((top + i) * vxcount + left) * sizeof(tke::VertexStat);
							ranges[i].srcOffset = (i * xlength) * sizeof(tke::VertexStat);
						}
						stagingBuffer.unmap();
						tke::copyBuffer(stagingBuffer.v, vertex_buffer->v, ranges.size(), ranges.data());
					}
				}
			}
		}
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
