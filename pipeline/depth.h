const float NEAR = 0.1;
const float FAR = 1000.0;

float LinearDepthOrtho(float z)
{
	z = z * 0.5 + 0.5;
	return z * (FAR - NEAR) + NEAR;
}

float LinearDepthPerspective(float z)
{
	float a = (1.0 - FAR / NEAR) / 2.0 / FAR;
	float b = (1.0 + FAR / NEAR) / 2.0 / FAR;
	return 1.0 / (a * z + b);
}
