R""(
#version 430

//attribute vec3 in_xyz;
//attribute vec2 in_uv0;
in vec3 in_xyz;
in vec2 in_uv0;

out vec2 uv0;

void main(void)
{
	uv0 = in_uv0;
	gl_Position = vec4(in_xyz, 1.0);
}

)""
