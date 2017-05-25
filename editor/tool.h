#ifndef __TOOL__
#define __TOOL__

#include "../src/transformer.h"
#include "../src/render.h"

struct Tool
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

	tke::Transformer::Type m_type;
	tke::Transformer::Axis m_axis = tke::Transformer::Axis::eNull;
	tke::Transformer *m_pTransformer = nullptr;

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
	void render(VkCommandBuffer);
	glm::vec4 getColor(int which);
	float rotate(glm::vec3 r, int axis);
	void setType(tke::Transformer::Type type);
};

void initTransformTool(VkRenderPass renderPass, int subpassIndex);

extern Tool *currentTool;
extern TransformTool transformTool;

void setTool(Tool *pTool);

#endif