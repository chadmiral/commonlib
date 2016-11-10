//basic fresnel
float fresnel_basic(float n_dot_v, float sharpness, float multiplier, float offset)
{
  return clamp(multiplier * (1.0 - pow(n_dot_v, sharpness)) + offset, 0.0, 1.0);
}
