#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Renderpass;
		struct Descriptorsetlayout;

		enum VertexAttributeType
		{
			VertexAttributeFloat,
			VertexAttributeFloat2,
			VertexAttributeFloat3,
			VertexAttributeFloat4,
			VertexAttributeByte4
		};

		enum PrimitiveTopology
		{
			PrimitiveTopologyPointList,
			PrimitiveTopologyLineList,
			PrimitiveTopologyLineStrip,
			PrimitiveTopologyTriangleList,
			PrimitiveTopologyTriangleStrip,
			PrimitiveTopologyTriangleFan,
			PrimitiveTopologyLineListWithAdjacency,
			PrimitiveTopologyLineStripWithAdjacency,
			PrimitiveTopologyTriangleListWithAdjacency,
			PrimitiveTopologyTriangleStripWithAdjacency,
			PrimitiveTopologyPatchList,
		};

		enum PolygonMode
		{
			PolygonModeFill,
			PolygonModeLine,
			PolygonModePoint
		};

		enum CullMode
		{
			CullModeNone,
			CullModeFront,
			CullModeBack,
			CullModeFrontAndback,
		};

		enum BlendFactor
		{
			BlendFactorZero,
			BlendFactorOne,
			BlendFactorSrcColor,
			BlendFactorOneMinusSrcColor,
			BlendFactorDstColor,
			BlendFactorOneMinusDstColor,
			BlendFactorSrcAlpha,
			BlendFactorOneMinusSrcAlpha,
			BlendFactorDstAlpha,
			BlendFactorOneMinusDstAlpha,
			BlendFactorConstantColor,
			BlendFactorOneMinusConstantColor,
			BlendFactorConstantAlpha,
			BlendFactorOneMinusConstantAlpha,
			BlendFactorSrcAlphaSaturate,
			BlendFactorSrc1Color,
			BlendFactorOneMinusSrc1Color,
			BlendFactorSrc1Alpha,
			BlendFactorOneMinusSrc1Alpha
		};

		enum DynamicState
		{
			DynamicStateViewport,
			DynamicStateScissor,
			DynamicStateLineWidth,
			DynamicStateDepthBias,
			DynamicStateBlendConstants,
			DynamicStateDepthBounds,
			DynamicStateStencilCompareMask,
			DynamicStateStencilWriteMask,
			DynamicStateStencilReference
		};

		struct PipelinelayoutPrivate;

		struct Pipelinelayout
		{
			PipelinelayoutPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void clear_descriptorsetlayouts();
			FLAME_GRAPHICS_EXPORTS void add_descriptorsetlayout(Descriptorsetlayout *descriptorsetlayout);
			FLAME_GRAPHICS_EXPORTS void clear_pushconstants();
			FLAME_GRAPHICS_EXPORTS void add_pushconstant(int offset, int size, ShaderType shader_type);
			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Pipelinelayout *create_pipelinelayout(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_pipelinelayout(Device *d, Pipelinelayout *p);

		struct PipelinePrivate;

		struct Pipeline
		{
			PipelinePrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void set_size(int cx, int cy);
			FLAME_GRAPHICS_EXPORTS void set_vertex_attributes(const std::initializer_list
				<std::initializer_list<VertexAttributeType>> &attributes);
			FLAME_GRAPHICS_EXPORTS void set_patch_control_points(int v);
			FLAME_GRAPHICS_EXPORTS void set_depth_test(bool v);
			FLAME_GRAPHICS_EXPORTS void set_depth_write(bool v);
			FLAME_GRAPHICS_EXPORTS void set_depth_clamp(bool v);
			FLAME_GRAPHICS_EXPORTS void set_primitive_topology(PrimitiveTopology v);
			FLAME_GRAPHICS_EXPORTS void set_polygon_mode(PolygonMode v);
			FLAME_GRAPHICS_EXPORTS void set_cull_mode(CullMode v);
			FLAME_GRAPHICS_EXPORTS void set_output_attachment_count(int count);
			FLAME_GRAPHICS_EXPORTS void set_blend_state(int index, bool enable, 
				BlendFactor src_color = BlendFactorOne, BlendFactor dst_color = BlendFactorZero, 
				BlendFactor src_alpha = BlendFactorOne, BlendFactor dst_alpha = BlendFactorZero);
			FLAME_GRAPHICS_EXPORTS void set_dynamic_state(const std::initializer_list<DynamicState> &states);
			FLAME_GRAPHICS_EXPORTS void add_shader(const std::string &filename,
				const std::initializer_list<std::string> &defines);
			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Pipeline *create_pipeline(Device *d, Renderpass *renderpass, int subpass_index);
		FLAME_GRAPHICS_EXPORTS void destroy_pipeline(Device *d, Pipeline *p);
	}
}
