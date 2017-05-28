#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

void main()
{
	gl_Position = vec4(vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2) * 2.0 - 1.0, 0, 1);
}