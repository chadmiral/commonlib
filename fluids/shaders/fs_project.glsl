R""(
#version 430

layout(location = 0) in vec2 uv0;

out vec4 frag_color;

void main()
{
  frag_color = vec4(uv0.s, uv0.t, 0.0, 1.0);
}
)""
