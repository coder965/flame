float LinearDepthOrtho(float z, float near, float far)
{
	z = z * 0.5 + 0.5;
	return z * (far - near) + near;
}

float LinearDepthPerspective(float z, float near, float far)
{
	float a = (1.0 - far / near) / 2.0 / far;
	float b = (1.0 + far / near) / 2.0 / far;
	return 1.0 / (a * z + b);
}
