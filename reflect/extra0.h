#define EXTRA "#include \"render.h\"\n \
			   #define PipelineAbstract PipelineAbstract<StageAbstract>\n \
			   #define DrawActionAbstract DrawActionAbstract<Drawcall>\n \
			   #define RenderPassAbstract RenderPassAbstract<Attachment, Dependency, DrawAction>\n \
			   #define RendererAbstract RendererAbstract<RenderPass>\n"