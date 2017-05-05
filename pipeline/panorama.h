vec2 panorama(vec3 v)
{
	return vec2(0.5 + 0.5 * atan(v.x, -v.z) / PI, acos(v.y) / PI);
}

vec3 inversePanorama(vec2 tc)
{
	float y = cos(tc.y * PI);
	float oneMinusY2 = 1.0 - y * y;
	float alpha = (tc.x + 0.25) * 2 * PI;
	return normalize(vec3(cos(alpha) * oneMinusY2, y, sin(alpha) * oneMinusY2));
}
