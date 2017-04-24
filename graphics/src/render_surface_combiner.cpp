#include "render_surface_combiner.h"

using namespace Graphics;
using namespace Math;

RenderSurfaceCombiner::RenderSurfaceCombiner()
{
 index_data[0] = 0;
 index_data[1] = 1;
 index_data[2] = 2;
 index_data[3] = 3;

 vertex_data[0].x =  -1.0f;
 vertex_data[0].y =  -1.0f;
 vertex_data[0].z =   0.0f;
 vertex_data[0].u =   0.0f;
 vertex_data[0].v =   0.0f;

 vertex_data[1].x =   1.0f;
 vertex_data[1].y =  -1.0f;
 vertex_data[1].z =   0.0f;
 vertex_data[1].u =   1.0f;
 vertex_data[1].v =   0.0f;

 vertex_data[2].x =   1.0f;
 vertex_data[2].y =   1.0f;
 vertex_data[2].z =   0.0f;
 vertex_data[2].u =   1.0f;
 vertex_data[2].v =   1.0f;

 vertex_data[3].x =  -1.0f;
 vertex_data[3].y =   1.0f;
 vertex_data[3].z =   0.0f;
 vertex_data[3].u =   0.0f;
 vertex_data[3].v =   1.0f;

 vbo = 0;
 ibo = 0;

 //lut3D = NULL;
 //vignette = NULL;
 shader = NULL;
}

RenderSurfaceCombiner::~RenderSurfaceCombiner()
{
  deinit();
}


void RenderSurfaceCombiner::set_shader_names(std::string vs, std::string fs)
{
  vertex_shader_name = vs;
  fragment_shader_name = fs;
}

void RenderSurfaceCombiner::init()
{
  if (!shader)
  {
    //TODO: this is bad cause we never deallocate this shader!
    shader = new Shader;
    shader->set_shader_filenames(vertex_shader_name, fragment_shader_name);
    shader->load_link_and_compile();
  }
  mat.set_shader(shader);

  gpu_texel_size.set_name("texel_size");
  gpu_texel_size.set_var(Float2(1.0f / (float)fbo_res[0], 1.0f / (float)fbo_res[1]));
  mat.add_uniform_var(&gpu_texel_size);

  //TODO: generalize this
  //mat.add_texture(vignette, "vignette_tex");

  _xyz_attrib.set_loc(shader, "in_xyz", sizeof(RenderSurfaceVert), 3, 0 * sizeof(float));
  _uv0_attrib.set_loc(shader, "in_uv0", sizeof(RenderSurfaceVert), 2, 3 * sizeof(float));

  mat.add_vertex_attrib(&_xyz_attrib);
  mat.add_vertex_attrib(&_uv0_attrib);

  mat.enable_backface_culling(false);
  mat.enable_depth_read(false);
  mat.enable_depth_write(false);
  mat.enable_blending(false);
  mat.enable_lighting(false);

  mat.init();

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 4, index_data, GL_STATIC_DRAW);
}

void RenderSurfaceCombiner::deinit()
{

}

void RenderSurfaceCombiner::render()
{
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClearDepth(1.0f);
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  //Shader *shader = mat.get_shader();

  mat.render();
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);

  mat.cleanup();
}
