#define EXTRA \
"#include \"../src/core/render.abstract.h\"\n \
#include \"../src/core/utils.h\"\n \
struct Stage;\n \
struct Pipeline;\n \
#define PipelineAbstract PipelineAbstract<Stage>\n"

const std::string my_skip[] = {
	"DrawcallAbstract",
	"DrawActionAbstract",
	"AttachmentAbstract",
	"DependencyAbstract",
	"RenderPassAbstract",
	"RendererAbstract" 
};

#define SKIP my_skip

#define INPUT "../src/core/render.abstract.h"

#define OUTPUT "../pipeline_editor/reflect.cpp"