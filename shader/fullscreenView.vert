#include "resolution.h"
#include "fovy.h"

layout(location = 0) out vec3 outViewDir;

void main()
{
	vec2 c = vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2) * 2.0 - 1.0;
	gl_Position = vec4(c, 0, 1);
	outViewDir = vec3(c * vec2(TAN_HF_FOVY * (float(RES_CX) / float(RES_CY)), -TAN_HF_FOVY), -1.0);
}
