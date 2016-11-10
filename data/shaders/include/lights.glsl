//vec4 transformed_sun_pos = gl_ModelViewMatrix * vec4(sun_pos, 1.0);
vec4 transform_light(vec3 light_pos)
{
  return gl_ModelViewMatrix * vec4(light_pos, 0.0);
}

//
// standard Lambertian diffuse lighting
//
vec3 diffuse_lambertian(vec3 normal, vec3 light_dir, vec3 surface_rgb, vec3 light_rgb)
{
  float n_dot_l = max(dot(normal, light_dir), 0.0);
  return n_dot_l * surface_rgb * light_rgb;
}

vec3 specular_phong(vec3 normal, vec3 light_dir, vec3 view_dir,
                    vec3 surface_rgb, vec3 light_rgb, float shininess)
{
  vec3 h = normalize(light_dir + view_dir);
  float h_dot_n = max(dot(h, normal), 0.0);
  return light_rgb * surface_rgb * pow(h_dot_n, shininess);
}
