R""(
#version 430

layout(location = 0) in vec2 uv0;

uniform in sampler2D prev_tex;
uniform in sampler2D curr_tex;
uniform in float dt;

out vec4 frag_color;

void main()
{
/*
  int size = (N + 2) * (N + 2);
  for(int i = 0; i < size; i++)
  {
    for(int j = a; j < b; j++)
    {
      _x[i].data[j] += dt * _s[i].data[j];
    }
  }
  */

  prev_tex + dt * curr_tex;

  vec4 prev = texture2D(prev_tex, uv0);
  frag_color = vec4(prev.r, uv0.x, uv0.y, 1.0);
}
)""
