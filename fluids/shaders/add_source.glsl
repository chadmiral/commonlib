R""(
#version 430

uniform writeonly image2D dest_tex;
layout (local_size_x = 1, local_size_y = 1) in;

void main()
{
  ivec2 store_pos = ivec2(gl_GlobalInvocationID.xy);
  imageStore(dest_tex, store_pos, vec4(1.0, 0.0, 0.0, 0.0));
}
)""
