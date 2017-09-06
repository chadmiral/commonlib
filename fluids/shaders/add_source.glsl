R""(
#version 430

layout (binding=0, r32f) readonly uniform image2D prev_tex;
layout (binding=1, r32f) writeonly uniform image2D curr_tex;

uniform writeonly image2D dest_tex;
layout (local_size_x = 16, local_size_y = 16) in;

void main()
{
  ivec2 store_pos = ivec2(gl_GlobalInvocationID.xy);
  imageStore(dest_tex, store_pos, vec4(0.0, 1.0, 0.0, 0.0));
}
)""
