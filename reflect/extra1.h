#define EXTRA "#include \"../src/core/render.abstract.h\"\n \
			   #include \"../src/core/utils.h\"\n \
			   #include \"shader.hpp\"\n \
			   #define PipelineAbstract PipelineAbstract<Stage>\n \
			   #define DrawActionAbstract DrawActionAbstract<Drawcall>\n \
			   #define RenderPassAbstract RenderPassAbstract<Attachment, Dependency, DrawAction>\n \
			   #define RendererAbstract RendererAbstract<RenderPass>\n"