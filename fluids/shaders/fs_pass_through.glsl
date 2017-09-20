R""(
#version 430

layout(location = 0) in vec2 uv0;

uniform in sampler2D tex;
out vec4 frag_color;

void main()
{
  frag_color = texture2D(prev_tex, uv0);
}
)""
