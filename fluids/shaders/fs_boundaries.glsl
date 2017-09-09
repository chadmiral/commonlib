R""(
#version 430

layout(location = 0) in vec2 uv0;

out vec4 frag_color;

void main()
{
  frag_color = vec4(1.0f, uv0.x, uv0.y, 1.0);
}
)""
