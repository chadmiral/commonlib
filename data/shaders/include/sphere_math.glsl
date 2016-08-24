float sphere_intersect(in vec3 ray_pos, in vec3 ray_dir, in vec4 sphere_pos)
{
	vec3 oc = ray_pos - sphere_pos.xyz;
	float b = dot(oc, ray_dir);
	float c = dot(oc, oc) - sphere_pos.w * sphere_pos.w; //sphere_pos.w = radius
	float h = b * b - c;
	if(h < 0.0)
	{
		return -1.0;
	}
	return -b - sqrt(h);
}
