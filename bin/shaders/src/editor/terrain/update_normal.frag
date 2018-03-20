layout(push_constant) uniform PushConstant
{
	int cx;
	int cy;
	int map_cx;
	int map_cy;
}pc;

layout(binding = 0) uniform HEIGHT
{
	float v[];
}u_height;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

int pitch;

float get_height(float x, float y)
{
	int X = (int)x;
	int Y = (int)y;
	return u_height[Y * pitch + X];
}

float get_height_uv(float xx, float yy)
{
	float xx = (float)pc.x * inTexcoord.x;
	float yy = (float)pc.y * inTexcoord.y;
	float x = floor(xx);
	float y = floor(yy);
	xx = fract(xx);
	yy = fract(yy);

	return mix(mix(get_height(x, y), get_height(x + 1, y), xx) +
		mix(get_height(x, y + 1), get_height(x + 1, y + 1), xx), yy);
}

void main()
{
	pitch = pc.x + 1;

	float xx = (float)pc.x * inTexcoord.x;
	float yy = (float)pc.y * inTexcoord.y;
	vec2 offset = vec2(1.0 / pc.map_cx, 1.0 / pc.map_cy);

	float hC = get_height_uv(xx, yy);
	float hL = get_height_uv(xx - offset.x, yy);
	float hR = get_height_uv(xx + offset.x, yy);
	float hU = get_height_uv(xx, yy - offset.y);
	float hD = get_height_uv(xx, yy + offset.y);

	float v0 = vec3(0.5, (hC + hR) * 0.5, 0.0) - vec3(-0.5, (hC + hL) * 0.5, 0.0);
	float v1 = vec3(0.0, (hC + hD) * 0.5, 0.5) - vec3(0.0, (hC + hU) * 0.5, -0.5);

	outColor = vec4(normalize(-cross(v0, v1)), hC);
}
