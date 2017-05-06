#define EXTRA \
"#include \"render.abstract.h\"\n \
#include \"utils.h\"\n \
#include \"render.h\"\n \
#define PipelineAbstract PipelineAbstract<StageAbstract>\n \
#define DrawActionAbstract DrawActionAbstract<Drawcall>\n \
#define RenderPassAbstract RenderPassAbstract<Attachment, Dependency, DrawAction>\n \
#define RendererAbstract RendererAbstract<RenderPass>\n"

#define INPUT "../src/core/render.abstract.h"

#define OUTPUT "../src/core/reflect.cpp"