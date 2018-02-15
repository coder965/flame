float LinearDepthOrtho(float z, float depth_near, float depth_far)
{
	z = z * 0.5 + 0.5;
	return z * (depth_far - depth_near) + depth_near;
}

float LinearDepthPerspective(float z, float depth_near, float depth_far)
{
	float a = (1.0 - depth_far / depth_near) * 0.5 / depth_far;
	float b = (1.0 + depth_far / depth_near) * 0.5 / depth_far;
	return 1.0 / (a * z + b);
}
