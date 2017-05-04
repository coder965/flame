#define EXTRA \
"#include \"../src/core/render.abstract.h\"\n \
#include \"../src/core/utils.h\"\n \
#include \"../render_editor/render.h\"\n \
#define PipelineAbstract PipelineAbstract<StageAbstract>\n \
#define DrawActionAbstract DrawActionAbstract<Drawcall>\n \
#define RenderPassAbstract RenderPassAbstract<Attachment, Dependency, DrawAction>\n \
#define RendererAbstract RendererAbstract<RenderPass>\n"

#define OUTPUT "../render_editor/reflect.cpp"