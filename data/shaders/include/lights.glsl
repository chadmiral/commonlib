//vec4 transformed_sun_pos = gl_ModelViewMatrix * vec4(sun_pos, 1.0);
vec4 transform_light(vec3 light_pos)
{
  return gl_ModelViewMatrix * vec4(light_pos, 0.0);
}
