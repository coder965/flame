#include "editor.h"
#include "../pickUp.h"
#include "../model.general.h"
#include "../../core/scene.h"

namespace tke
{
	void Tool::attach() {}
	void Tool::release() {}
	void Tool::update() {}
	bool Tool::mouseDown(int x, int y) { return false; }
	void Tool::mouseUp() {}
	void Tool::mouseMove() {}

	namespace TransformToolData
	{
		struct VertData
		{
			glm::mat4 matrixMVP[4];
			glm::mat4 matrixNormal[4];
		};

		struct FragData
		{
			glm::vec4 color[4];
		};

		UniformBuffer vertBuffer;
		UniformBuffer fragBuffer;
		UniformBuffer rectBuffer;

		Pipeline pipeline;
		Pipeline pickUpPipeline;
		Pipeline rectPipeline;

		bool needUpdataUniformBuffer = true;

		void init(VkRenderPass renderPass, int subpassIndex)
		{
			vertBuffer.create(sizeof VertData);
			fragBuffer.create(sizeof FragData);
			rectBuffer.create(sizeof glm::vec4);
			globalResource.setBuffer(&vertBuffer, "Editor.Tool.Transform.Vert.UniformBuffer");
			globalResource.setBuffer(&fragBuffer, "Editor.Tool.Transform.Frag.UniformBuffer");
			globalResource.setBuffer(&rectBuffer, "Editor.Tool.Transform.Rect.UniformBuffer");

			pipeline.create("../shader/tool/transform/transform.xml", &vertexInputState, 0, 0, renderPass, subpassIndex);
			rectPipeline.create("../shader/tool/transform/rect.xml", &zeroVertexInputState, 0, 0, renderPass, subpassIndex);
			pickUpPipeline.create("../shader/tool/transform/pickUp.xml", &vertexInputState, 0, 0, renderPass, subpassIndex);
		}
	}

	TransformTool::TransformTool()
	{

		for (int i = 0; i < 3; i++)
		{
			m_flag[i][0] = 1;
			m_flag[i][1] = 2;
			m_flag[i][2] = 4;
		}
	}

	void TransformTool::attach()
	{
		postRedrawRequest();
	}

	void TransformTool::release()
	{
		postRedrawRequest();
	}

	void TransformTool::update()
	{
		auto pTrans = selectTransformer();
		if (pTrans != m_pTransformer)
		{
			m_pTransformer = pTrans;
			ZeroMemory(m_flag, sizeof(m_flag));
			// 0 means ban
			switch (selectType)
			{
			case SelectType::eLight:
			case SelectType::eRigidbody:
			case SelectType::eJoint:
				m_flag[0][0] = XFLAG;
				m_flag[0][1] = YFLAG;
				m_flag[0][2] = ZFLAG;

				m_flag[1][0] = XFLAG;
				m_flag[1][1] = YFLAG;
				m_flag[1][2] = ZFLAG;
				break;
			case SelectType::eObject:
			case SelectType::eTerrain:
				m_flag[0][0] = XFLAG;
				m_flag[0][1] = YFLAG;
				m_flag[0][2] = ZFLAG;

				m_flag[1][0] = XFLAG;
				m_flag[1][1] = YFLAG;
				m_flag[1][2] = ZFLAG;

				m_flag[2][0] = XFLAG;
				m_flag[2][1] = YFLAG;
				m_flag[2][2] = ZFLAG;
				break;
			case SelectType::eShape:
				switch (selectShape()->type)
				{
				case Shape::Type::eBox:
					m_flag[0][0] = XFLAG;
					m_flag[0][1] = YFLAG;
					m_flag[0][2] = ZFLAG;

					m_flag[1][0] = XFLAG;
					m_flag[1][1] = YFLAG;
					m_flag[1][2] = ZFLAG;

					m_flag[2][0] = XFLAG;
					m_flag[2][1] = YFLAG;
					m_flag[2][2] = ZFLAG;
					break;
				case Shape::Type::eSphere:
					m_flag[0][0] = XFLAG;
					m_flag[0][1] = YFLAG;
					m_flag[0][2] = ZFLAG;

					m_flag[2][0] = XFLAG | YFLAG | ZFLAG;
					m_flag[2][1] = XFLAG | YFLAG | ZFLAG;
					m_flag[2][2] = XFLAG | YFLAG | ZFLAG;
					break;
				case Shape::Type::eCapsule:
					m_flag[0][0] = XFLAG;
					m_flag[0][1] = YFLAG;
					m_flag[0][2] = ZFLAG;

					m_flag[1][0] = XFLAG;
					m_flag[1][1] = YFLAG;
					m_flag[1][2] = ZFLAG;

					m_flag[2][0] = XFLAG | ZFLAG;
					m_flag[2][1] = YFLAG;
					m_flag[2][2] = XFLAG | ZFLAG;
					break;
				case Shape::Type::ePlane:
					m_flag[0][0] = XFLAG;
					m_flag[0][1] = YFLAG;
					m_flag[0][2] = ZFLAG;

					m_flag[1][1] = YFLAG;

					m_flag[2][0] = XFLAG;
					m_flag[2][2] = ZFLAG;
					break;
				}
				break;
			}
			postRedrawRequest();
			TransformToolData::needUpdataUniformBuffer = true;
		}
		if (m_pTransformer)
		{
			{
				auto data = glm::vec4(mouseX, mouseY, 5.f / resCx, 5.f / resCy);
				TransformToolData::rectBuffer.update(&data, &stagingBuffer);
			}

			if (m_pTransformer->m_changed || scene->camera.m_changed) TransformToolData::needUpdataUniformBuffer = true;

			if (TransformToolData::needUpdataUniformBuffer)
			{
				glm::mat3 mat_vr = glm::mat3(scene->camera.getMatInv());
				glm::mat4 mat_vt = glm::translate(scene->camera.getCoord());
				glm::vec4 p = scene->camera.getMatInv() * glm::vec4(m_pTransformer->getCoord(), 1);
				m_projCoord = *pMatProj * p;
				m_projCoord /= m_projCoord.w;
				glm::vec3 np = glm::vec3(p) * 5.f / glm::length(p);
				switch (m_type)
				{
				case Transformer::Type::eMove:
					m_matAxis[0] = glm::translate(np) * glm::mat4(mat_vr);
					m_matAxis[1] = m_matAxis[0] * glm::rotate(90.f, glm::vec3(0.f, 0.f, 1.f));
					m_matAxis[2] = m_matAxis[0] * glm::rotate(-90.f, glm::vec3(0.f, 1.f, 0.f));
					break;
				case Transformer::Type::eAsixRotate:
					m_matAxis[1] = glm::translate(np) * glm::mat4(mat_vr * m_pTransformer->getAxis());
					m_matAxis[0] = m_matAxis[1] * glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f));
					m_matAxis[2] = m_matAxis[1] * glm::rotate(90.f, glm::vec3(1.f, 0.f, 0.f));
					break;
				case Transformer::Type::eScale:
				{
					m_matAxis[0] = glm::translate(np) * glm::mat4(mat_vr * m_pTransformer->getAxis());
					m_matAxis[1] = m_matAxis[0] * glm::rotate(90.f, glm::vec3(0.f, 0.f, 1.f));
					m_matAxis[2] = m_matAxis[0] * glm::rotate(-90.f, glm::vec3(0.f, 1.f, 0.f));
				}
				break;
				}

				TransformToolData::VertData vertData;
				TransformToolData::FragData fragData;

				for (int i = 0; i < 3; i++)
				{
					fragData.color[i] = getColor(i);
					if (m_type == Transformer::Type::eMove)
					{
						vertData.matrixMVP[i] = (*pMatProj) * m_matAxis[i];
						vertData.matrixNormal[i] = glm::mat4(glm::mat3(m_matAxis[i]));
					}
					else if (m_type == Transformer::Type::eAsixRotate)
					{
						vertData.matrixMVP[i] = (*pMatProj) * m_matAxis[i];
						vertData.matrixNormal[i] = glm::mat4(glm::mat3(m_matAxis[i]));
					}
					else if (m_type == Transformer::Type::eScale)
					{
						glm::mat4 mat0 = m_matAxis[i], mat1 = m_matAxis[i];
						if (m_flag[2][i] & AXIS_FLAG[(int)m_axis])
						{
							mat0 = mat0 * glm::scale(m_scaleNow, 1.f, 1.f);
							mat1 = mat1 * glm::translate(0.9f * (m_scaleNow - 1.f), 0.f, 0.f);
							fragData.color[3] = getColor(i);
							vertData.matrixMVP[3] = (*pMatProj) * mat1;
							vertData.matrixNormal[3] = glm::mat4(glm::mat3(mat1));
						}
						vertData.matrixMVP[i] = (*pMatProj) * mat0;
						vertData.matrixNormal[i] = glm::mat4(glm::mat3(mat0));
					}

					TransformToolData::vertBuffer.update(&vertData, &stagingBuffer);
					TransformToolData::fragBuffer.update(&fragData, &stagingBuffer);
				}
			}

			TransformToolData::needUpdataUniformBuffer = false;
		}
	}

	bool TransformTool::mouseDown(int x, int y)
	{
		if (!m_pTransformer)
			return false;

		auto index = pickUp(x, y, 1, 1, [](VkCommandBuffer cmd, void *userData) {
			auto pTool = (TransformTool*)userData;

			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, &scene->vertexBuffer.m_buffer, offsets);
			vkCmdBindIndexBuffer(cmd, scene->indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TransformToolData::pickUpPipeline.m_pipeline);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TransformToolData::pickUpPipeline.m_pipelineLayout, 0, 1, &TransformToolData::pickUpPipeline.m_descriptorSet, 0, nullptr);

			if (pTool->m_type == Transformer::Type::eMove)
			{
				auto pModel = arrowModel;
				vkCmdDrawIndexed(cmd, pModel->indices.size(), 3, pModel->indiceBase, pModel->vertexBase, 0);
			}
			else if (pTool->m_type == Transformer::Type::eAsixRotate)
			{
				auto pModel = torusModel;
				vkCmdDrawIndexed(cmd, pModel->indices.size(), 3, pModel->indiceBase, pModel->vertexBase, 0);
			}
			else if (pTool->m_type == Transformer::Type::eScale)
			{
				auto pModel = hamerModel;
				for (int i = 0; i < 3; i++)
				{
					vkCmdDrawIndexed(cmd, pModel->renderGroups[0].indiceCount, 1, pModel->indiceBase + pModel->renderGroups[0].indiceBase, hamerModel->vertexBase, i);
					vkCmdDrawIndexed(cmd, pModel->renderGroups[1].indiceCount, 1, pModel->indiceBase + pModel->renderGroups[1].indiceBase, hamerModel->vertexBase, pTool->m_flag[2][i] & pTool->AXIS_FLAG[(int)pTool->m_axis] ? 3 : i);
				}
			}
		}, this);

		if (index == 0) return false;
		index--;

		if (index != 3 && m_axis != Transformer::Axis(index))
		{
			postRedrawRequest();
			TransformToolData::needUpdataUniformBuffer = true;
			m_axis = Transformer::Axis(index);
		}

		m_prevCoord = glm::unProject(glm::vec3(mouseX, mouseY, (m_projCoord.z + 1.0f) / 2.0f), scene->camera.getMat(), *pMatProj, glm::vec4(-1.f, -1.f, 2.f, 2.f));;
		m_prevCoord -= m_pTransformer->getCoord();

		switch (m_type)
		{
		case Transformer::Type::eAsixRotate:
			m_rotateOffset = rotate(m_prevCoord, (int)m_axis);
			break;
		case Transformer::Type::eScale:
			m_scaleOriginal = m_pTransformer->getScale()[(int)m_axis];
			m_scaleLength = glm::dot(m_prevCoord, m_pTransformer->getAxis()[(int)m_axis]);
			m_scaleNow = 1.0f;
			break;
		}

		beginRecordTransformHistory();
		return true;
	}

	void TransformTool::mouseUp()
	{
		m_scaleNow = 1.0f;

		endRecordTransformHistory();

		TransformToolData::needUpdataUniformBuffer = true;
	}

	void TransformTool::mouseMove()
	{
		if (!m_pTransformer || m_axis == Transformer::Axis::eNull)
			return;

		glm::vec3 result = glm::unProject(glm::vec3(mouseX, mouseY, (m_projCoord.z + 1.0f) / 2.0f), scene->camera.getMat(), *pMatProj, glm::vec4(-1.f, -1.f, 2.f, 2.f));

		switch (m_type)
		{
		case Transformer::Type::eMove:
		{
			result -= m_prevCoord;
			glm::vec3 coord = m_pTransformer->getCoord();
			for (int i = 0; i < 3; i++)
			{
				if (m_flag[0][i] & AXIS_FLAG[(int)m_axis])
					coord[i] = result[i];
			}
			moveTransformer(selectType, m_pTransformer, coord);
		}
			break;
		case Transformer::Type::eAsixRotate:
		{
			float rotate_ang = rotate(result - m_pTransformer->getCoord(), (int)m_axis);
			auto ang = rotate_ang - m_rotateOffset;
			for (int i = 0; i < 3; i++)
			{
				if (m_flag[1][i] & AXIS_FLAG[(int)m_axis])
					rotateTransformerAxis(selectType, m_pTransformer, Transformer::Axis(i), ang);
			}
		}
			break;
		case Transformer::Type::eScale:
		{
			m_scaleNow = glm::dot(result - m_pTransformer->getCoord(), m_pTransformer->getAxis()[(int)m_axis]) / m_scaleLength; // MAYBE BUG !!!!
			float value = m_scaleOriginal * m_scaleNow;
			glm::vec3 scale = m_pTransformer->getScale();
			for (int i = 0; i < 3; i++)
			{
				if (m_flag[2][i] & AXIS_FLAG[(int)m_axis])
					scale[i] = value;
			}
			scaleTransformer(selectType, m_pTransformer, scale);
		}
			break;
		}
	}

	glm::vec4 TransformTool::getColor(int which)
	{
		glm::vec3 color = glm::vec3(0);
		if ((int)m_axis == which)
			color = glm::vec3(1, 1, 0);
		else if (which == (int)Transformer::Axis::eX)
			color = glm::vec3(1, 0, 0);
		else if (which == (int)Transformer::Axis::eY)
			color = glm::vec3(0, 1, 0);
		else if (which == (int)Transformer::Axis::eZ)
			color = glm::vec3(0, 0, 1);
		int v = (int)m_type;
		if (v == 3) v = 1;
		if (!(m_flag[v][which] & AXIS_FLAG[which])) color *= 0.2f;
		return glm::vec4(color, 1.f);
	}

	float TransformTool::rotate(glm::vec3 r, int iAxis)
	{
		glm::mat3 axis = m_pTransformer->getAxis();
		r = glm::transpose(glm::mat3(axis[(iAxis + 2) % 3], axis[iAxis], axis[(iAxis + 1) % 3])) * r; // MAYBE BUG !!!!!!
		return -glm::mod((float)(glm::degrees(atan2(r.z, r.x))), 360.0f);
	}

	void TransformTool::setType(Transformer::Type type)
	{
		if (type == m_type) return;
		m_type = type;
		if (type == Transformer::Type::eScale) m_scaleNow = 1.f;
		TransformToolData::needUpdataUniformBuffer = true;
		postRedrawRequest();
	}

	void TransformTool::render(VkCommandBuffer cmd)
	{
		if (!m_pTransformer) return;

		VkClearAttachment clearAttachment = {};
		clearAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		clearAttachment.colorAttachment = 0;
		clearAttachment.clearValue.depthStencil = { 1.f, 0 };
		VkClearRect clearRect = {};
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;
		clearRect.rect.extent.width = resCx;
		clearRect.rect.extent.height = resCy;
		vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TransformToolData::pipeline.m_pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TransformToolData::pipeline.m_pipelineLayout, 0, 1, &TransformToolData::pipeline.m_descriptorSet, 0, nullptr);

		if (m_type == Transformer::Type::eMove)
		{
			auto pModel = arrowModel;
			vkCmdDrawIndexed(cmd, pModel->indices.size(), 3, pModel->indiceBase, pModel->vertexBase, 0);
		}
		else if (m_type == Transformer::Type::eAsixRotate)
		{
			auto pModel = torusModel;
			vkCmdDrawIndexed(cmd, pModel->indices.size(), 3, pModel->indiceBase, pModel->vertexBase, 0);
		}
		else if (m_type == Transformer::Type::eScale)
		{
			auto pModel = hamerModel;
			for (int i = 0; i < 3; i++)
			{
				vkCmdDrawIndexed(cmd, pModel->renderGroups[0].indiceCount, 1, pModel->indiceBase + pModel->renderGroups[0].indiceBase, pModel->vertexBase, i);
				vkCmdDrawIndexed(cmd, pModel->renderGroups[1].indiceCount, 1, pModel->indiceBase + pModel->renderGroups[1].indiceBase, pModel->vertexBase, m_flag[2][i] & AXIS_FLAG[(int)m_axis] ? 3 : i);
			}
		}

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TransformToolData::rectPipeline.m_pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TransformToolData::rectPipeline.m_pipelineLayout, 0, 1, &TransformToolData::rectPipeline.m_descriptorSet, 0, nullptr);
		vkCmdDraw(cmd, 5, 1, 0, 0);
	}

	void initTransformTool(VkRenderPass renderPass, int subpassIndex)
	{
		TransformToolData::init(renderPass, subpassIndex);
	}
}
