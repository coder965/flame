#ifndef __TKE_EDITOR_TOOL__
#define __TKE_EDITOR_TOOL__

#include "..\..\core\transformer.h"
#include "..\..\core\render.h"

namespace tke
{
	struct Tool : Renderable
	{
		int cx = 0;
		int cy = 0;

		virtual void attach();
		virtual void release();
		virtual void update();
		virtual bool mouseDown(int x, int y);
		virtual void mouseUp();
		virtual void mouseMove();
	};

	struct TransformTool : Tool
	{
		const int XFLAG = 1;
		const int YFLAG = 2;
		const int ZFLAG = 4;
		const int AXIS_FLAG[3] = { XFLAG, YFLAG, ZFLAG };

		Transformer::Type m_type;
		Transformer::Axis m_axis = Transformer::Axis::eNull;
		Transformer *m_pTransformer = nullptr;

		char m_flag[3][3];

		glm::mat4 m_matAxis[3];

		glm::vec4 m_projCoord;
		glm::vec3 m_prevCoord;
		float m_rotateOffset;
		float m_scaleOriginal, m_scaleNow, m_scaleLength;

		TransformTool();
		virtual void attach() override;
		virtual void release() override;
		virtual void update() override;
		virtual bool mouseDown(int x, int y) override;
		virtual void mouseUp() override;
		virtual void mouseMove() override;
		virtual void render(VkCommandBuffer) override;
		glm::vec4 getColor(int which);
		float rotate(glm::vec3 r, int axis);
		void setType(Transformer::Type type);
	};

	void initTransformTool(VkRenderPass renderPass, int subpassIndex);
}

#endif