float rand2d(vec2 v)
{
	return fract(cos(v.x * (12.9898) + v.y * (4.1414)) * 43758.5453);
}

float noise2d(vec2 v)
{
	const uint SC = 250;

	v /= SC;
	vec2 vf = fract(v);
	vec2 vi = floor(v);

	float r0 = rand2d(vi);
	float r1 = rand2d(vi + vec2(1.0, 0.0));
	float r2 = rand2d(vi + vec2(0.0, 1.0));
	float r3 = rand2d(vi + vec2(1.0, 1.0));

	vec2 vs = 3.0 * vf * vf - 2.0 * vf * vf * vf;

	return mix(mix(r0, r1, vs.x),
			mix(r2, r3, vs.x),
			vs.y);
}

float fbm2d(vec2 v)
{
	float r = 0.0;

	float a = 1.0 / 3.0;
	for (int i = 0; i < 4; i++)
	{
		r += noise2d(v) * a;
		a /= 3.0;
		v *= 3.0;
	}

	return r;
}