#include <stddef.h>
#include <assert.h>
#include <iostream>
#include <math.h>
#include "gpu_hair_sim.h"
#include "math_utility.h"
#include "perlin.h"

using namespace std;
using namespace Math;
using namespace Graphics;
using namespace PerlinNoise;

static char *hair_sim_vs =
"#version 120\n\
\n\
void main(void) {\n\
  gl_TexCoord[0] = gl_MultiTexCoord0;\n\
  gl_TexCoord[1] = gl_MultiTexCoord1;\n\
  gl_Position = ftransform();\n\
}\n\
";

static char *hair_sim_fs =
"#version 120\n\
\n\
uniform sampler2D prev_pos_tex;\n\
uniform sampler2D force_tex;\n\
uniform sampler2D uv_tex;\n\
uniform vec4 constants;\n\
\n\
uniform float texel_size;\n\
\n\
// F = -k(x - d) (Hooke's Spring Law)\n\
void main()\n\
{\n\
  float dt = clamp(constants.x, 0.0, 0.02);\n\
  float k = constants.y;\n\
  float texel_size = constants.z;\n\
  float spring_length = constants.w;\n\
  float wind_strength = 0.5;\n\
  float drag = 0.1;\n\
\n\
  vec3 force_uvs = texture2D(uv_tex, gl_TexCoord[0].st).rgb;\n\
  vec4 prev_pos = texture2D(prev_pos_tex, gl_TexCoord[0].st);\n\
  vec4 parent_pos = texture2D(prev_pos_tex, gl_TexCoord[0].st - vec2(0.0, texel_size));\n\
  vec4 external_force = wind_strength * texture2D(force_tex, force_uvs.xy);\n\
\n\
  spring_length = spring_length * external_force.w * force_uvs.z; //scale by height\n\
\n\
  vec3 v_spring = prev_pos.xyz - parent_pos.xyz;\n\
  float r = length(v_spring);\n\
\n\
  vec3 spring_force = vec3(0.0, 0.0, 0.0);\n\
  if (r > 0.0)\n\
  {\n\
    spring_force = -k * (v_spring / r) * (r - spring_length);\n\
  }\n\
\n\
  vec3 final_force = dt * (external_force.xyz + spring_force);\n\
  if (gl_TexCoord[0].t < texel_size)\n\
  {\n\
    final_force = vec3(0.0, 0.0, 0.0);\n\
  }\n\
\n\
  gl_FragColor = prev_pos + vec4(final_force, 0.0);// + offset;\n\
}\n\
";

static char *hair_render_vs =
"#version 120\n\
\n\
uniform sampler2D hair_tex;\n\
uniform sampler2D uv_tex;\n\
uniform sampler2D color_tex;\n\
uniform vec3 sun_pos;\n\
uniform vec3 sun_diff_rgb;\n\
uniform vec3 light_amb_rgb;\n\
\n\
varying vec4 vertex_color;\n\
varying vec3 view_dir;\n\
varying vec3 light_dir;\n\
\n\
void main()\n\
{\n\
  gl_TexCoord[0] = gl_MultiTexCoord0;\n\
\n\
  view_dir = -vec3(gl_ProjectionMatrix[2][0], gl_ProjectionMatrix[2][1], gl_ProjectionMatrix[2][2]);\n\
\n\
  vec4 pos_offset = texture2D(hair_tex, gl_TexCoord[0].st);\n\
  vec3 uvs = texture2D(uv_tex, gl_TexCoord[0].st).rgb;\n\
\n\
  vec4 pos = gl_Vertex + pos_offset;\n\
  vec3 n = normalize(gl_NormalMatrix * pos.xyz);\n\
\n\
  light_dir = normalize(sun_pos - pos.xyz);\n\
  float n_dot_l = clamp(dot(n, light_dir), 0.0, 1.0);\n\
  vec3 diffuse_term = n_dot_l * sun_diff_rgb;\n\
  vec3 tint_color = texture2D(color_tex, uvs.xy).rgb;\n\
  vec3 ambient_term = light_amb_rgb;\n\
  vertex_color = vec4(diffuse_term * tint_color * gl_Color.rgb + ambient_term * tint_color, 1.0);\n\
\n\
  gl_Position = gl_ModelViewProjectionMatrix * pos;\n\
}\n\
";

static char *hair_render_fs =
"#version 120\n\
\n\
varying vec4 vertex_color;\n\
\n\
void main()\n\
{\n\
  //vec4 tint = texture2D(color_tex, gl_TexCoord[0].st);\n\
  gl_FragColor = vertex_color;\n\
}\n\
";

GPUHairSim::GPUHairSim()
{
  num_hairs = 4;
  num_segments = 4;
  hair_height = 1.0f;

  pos_fbo[0] = pos_fbo[1] = 0;
  pos_tex[0] = pos_tex[1] = 0;
  force_tex = 0;
  vbo = ibo = 0;
  verts = NULL;
  num_verts = 0;
  indices = NULL;
  num_indices = 0;
  force_tex_dim[0] = force_tex_dim[1] = 128;
  col_a = Float3(0.3f, 0.3f, 0.3f);
  col_b = Float3(1.0f, 1.0f, 1.0f);
  color_tex = NULL;
  internal_format = GL_RGBA_FLOAT32_ATI; //GL_RGBA32F;//

  fbo_indices[0] = 0;
  fbo_indices[1] = 1;
  fbo_indices[2] = 2;
  fbo_indices[3] = 3;

  fbo_verts[0].x =  -1.0f;
  fbo_verts[0].y =  -1.0f;
  fbo_verts[0].z =   0.0f;
  fbo_verts[0].u =   0.0f;
  fbo_verts[0].v =   0.0f;

  fbo_verts[1].x =   1.0f;
  fbo_verts[1].y =  -1.0f;
  fbo_verts[1].z =   0.0f;
  fbo_verts[1].u =   1.0f;
  fbo_verts[1].v =   0.0f;

  fbo_verts[2].x =   1.0f;
  fbo_verts[2].y =   1.0f;
  fbo_verts[2].z =   0.0f;
  fbo_verts[2].u =   1.0f;
  fbo_verts[2].v =   1.0f;

  fbo_verts[3].x =  -1.0f;
  fbo_verts[3].y =   1.0f;
  fbo_verts[3].z =   0.0f;
  fbo_verts[3].u =   0.0f;
  fbo_verts[3].v =   1.0f;

  num_fbo_verts = num_fbo_indices = 4;
  fbo_vbo = fbo_ibo = 0;

  spring_constant = 60.0f;

  sun_pos_xyz = Float3(10.0f, 10.0f, 2.0f);
  sun_diff_rgb = Float3(3.0f, 2.5f, 1.0f);
  sun_spec_rgb = Float3(1.0f, 1.0f, 0.0f);
  ambient_rgb = Float3(0.2f, 0.3f, 0.3f);

  render_shader = new Shader;
  sim_shader = new Shader;
}

GPUHairSim::~GPUHairSim()
{
  deinit();
  delete render_shader;
  delete sim_shader;
}

void GPUHairSim::init(Float3 *hair_pos, Float3 *hair_uvs)
{
  //1. create our textures
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  //create uv tex
  glGenTextures(1, &uv_tex);
  glBindTexture(GL_TEXTURE_2D, uv_tex);
  assert(glIsTexture(uv_tex) == GL_TRUE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,//GL_RGBA16F_ARB,
               num_hairs,           //u axis = hair index
               1,//num_segments,        //v axis = hair segment index
               0,
               GL_RGBA, //GL_RGBA16F_ARB,      //RGB = xyz (pos), A = ?
               GL_FLOAT,//GL_UNSIGNED_BYTE,
               NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  assert(glIsTexture(uv_tex) == GL_TRUE);

  //fill uv tex
  int pixel_idx = 0;
  GLfloat *pixels = new GLfloat[num_hairs * num_segments * 4];
  for(int k = 0; k < 1/*num_segments*/; k++)
  {
    float seg_pct = (float)k / (float)num_segments;
    for(int j = 0; j < num_hairs; j++)
    {
      pixels[pixel_idx++] = hair_uvs[j][0];
      pixels[pixel_idx++] = hair_uvs[j][1];
      pixels[pixel_idx++] = hair_uvs[j][2]; //height variance
      pixels[pixel_idx++] = 1.0f;           //put something useful here
    }
  }

  glTexSubImage2D(GL_TEXTURE_2D,
                  0,                //mip level
                  0,                //starting u coord
                  0,                //starting v coord
                  num_hairs,        //width of update rect
                  1,//num_segments,     //height of update rect
                  GL_RGBA,       //pixel format
                  GL_FLOAT,
                  pixels);          //pointer to pixel data

  delete pixels;

  //create a force texture
  glGenTextures(1, &force_tex);
  glBindTexture(GL_TEXTURE_2D, force_tex);

  assert(glIsTexture(force_tex) == GL_TRUE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,//GL_RGBA16F_ARB,
               force_tex_dim[0],
               force_tex_dim[1],
               0,
               GL_RGBA,
               GL_FLOAT,
               NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  assert(glIsTexture(force_tex) == GL_TRUE);

  //create 2 2D displacement textures to represet current and previous
  //frame displacement per hair
  glGenTextures(2, pos_tex);
  glGenFramebuffersEXT(2, pos_fbo);
  for(int i = 0; i < 2; i++)
  {
    GLuint pixel_mode = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, pos_tex[i]);
    assert(glIsTexture(pos_tex[i]) == GL_TRUE);

    //pixel_mode = GL_RGBA_FLOAT32_ATI;//GL_RGBA16F_ARB;

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internal_format,//GL_RGBA16F_ARB,
                 num_hairs,           //u axis = hair index
                 num_segments,        //v axis = hair segment index
                 0,
                 pixel_mode, //GL_RGBA16F_ARB,      //RGB = xyz (pos), A = ?
                 GL_FLOAT,//GL_UNSIGNED_BYTE,
                 NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    assert(glIsTexture(pos_tex[i]) == GL_TRUE);

    //fill textures w/ initial values (random points in [-1, 1], [-1, 1])
    int pixel_idx = 0;
    int num_bytes = (pixel_mode == GL_RGBA) ? 4 : 3;
    GLfloat *pixels = new GLfloat[num_hairs * num_segments * num_bytes];
    for(int k = 0; k < num_segments; k++)
    {
      float seg_pct = (float)k / (float)num_segments;
      for(int j = 0; j < num_hairs; j++)
      {
        pixels[pixel_idx++] = hair_pos[j][0];
        pixels[pixel_idx++] = hair_pos[j][1];//hair_height * seg_pct;
        pixels[pixel_idx++] = hair_pos[j][2];
        if(num_bytes == 4)
        {
          pixels[pixel_idx++] = 1.0f;
        }
      }
    }

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,                //mip level
                    0,                //starting u coord
                    0,                //starting v coord
                    num_hairs,        //width of update rect
                    num_segments,     //height of update rect
                    pixel_mode,       //pixel format
                    GL_FLOAT,
                    pixels);          //pointer to pixel data

    delete pixels;

    //create FBOs + buffers for full screen quad
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pos_fbo[i]);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pos_tex[i], 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }

  glGenBuffers(1, &fbo_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_verts), fbo_verts, GL_STATIC_DRAW);

  glGenBuffers(1, &fbo_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 4, fbo_indices, GL_STATIC_DRAW);

  //create our geometry buffers
  num_verts = num_hairs * (num_segments) * 2;
  num_indices = num_verts;
  verts = new HairVert[num_verts];
  indices = new unsigned int[num_indices];

  int v_idx = 0; int i_idx = 0;

  for(int i = 0; i < num_hairs; i++)
  {
    for(int j = 1; j < num_segments + 1; j++)
    {
      for(int k = j - 1; k <= j; k++)
      {
        float segment_pct = (float)(k) / (float)num_segments;
        verts[v_idx].x = 0.0f;
        verts[v_idx].y = 0.0f;
        verts[v_idx].z = 0.0f;
        verts[v_idx].r = lerp(col_a[0], col_b[0], segment_pct);
        verts[v_idx].g = lerp(col_a[1], col_b[1], segment_pct);
        verts[v_idx].b = lerp(col_a[2], col_b[2], segment_pct);
        verts[v_idx].u0 = (float)i / (float)num_hairs;
        verts[v_idx].v0 = segment_pct;

        indices[i_idx++] = v_idx++;
      }
    }
  }

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(HairVert) * num_verts, &verts[0].x, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * num_indices, indices, GL_STATIC_DRAW);

  //load textures and shaders
  render_shader->compile_and_link_from_source(hair_render_vs, hair_render_fs);
  render_mat.set_shader(render_shader);
  render_mat.init();

  sim_shader->compile_and_link_from_source(hair_sim_vs, hair_sim_fs);
  sim_mat.set_shader(sim_shader);
  sim_mat.init();

  //retrieve shader uniform locations
  sim_mat.render();
  Shader *shader = sim_mat.get_shader();
  uniform_locations[UNIFORM_SIM_CONSTANTS] = glGetUniformLocation(shader->gl_shader_program, "constants");
  uniform_locations[UNIFORM_SIM_POS_TEX] = glGetUniformLocation(shader->gl_shader_program, "prev_pos_tex");
  uniform_locations[UNIFORM_SIM_FORCE_TEX] = glGetUniformLocation(shader->gl_shader_program, "force_tex");
  uniform_locations[UNIFORM_SIM_UV_TEX] = glGetUniformLocation(shader->gl_shader_program, "uv_tex");

  render_mat.render();
  shader = render_mat.get_shader();
  uniform_locations[UNIFORM_RENDER_HAIR_TEX] = glGetUniformLocation(shader->gl_shader_program, "hair_tex");
  uniform_locations[UNIFORM_RENDER_UV_TEX] = glGetUniformLocation(shader->gl_shader_program, "uv_tex");
  uniform_locations[UNIFORM_RENDER_COLOR_TEX] = glGetUniformLocation(shader->gl_shader_program, "color_tex");
  uniform_locations[UNIFORM_RENDER_SUN_POS] = glGetUniformLocation(shader->gl_shader_program, "sun_pos");
  uniform_locations[UNIFORM_RENDER_DIFF] = glGetUniformLocation(shader->gl_shader_program, "sun_diff_rgb");
  uniform_locations[UNIFORM_RENDER_SPEC] = glGetUniformLocation(shader->gl_shader_program, "sun_spec_rgb");
  uniform_locations[UNIFORM_RENDER_AMBIENT] = glGetUniformLocation(shader->gl_shader_program, "light_amb_rgb");
  uniform_locations[UNIFORM_RENDER_CAM_DISTANCE] = glGetUniformLocation(shader->gl_shader_program, "cam_distance");

  glUseProgram(0);
}

void GPUHairSim::deinit()
{
  glDeleteTextures(2, pos_tex);
  glDeleteTextures(1, &force_tex);

  //tear down vbo, ibo
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
  if(verts) { delete verts; }
  if(indices) { delete indices; }

  pos_tex[0] = pos_tex[1] = force_tex = vbo = ibo = 0;
  verts = NULL; indices = NULL;
  num_verts = num_indices = 0;
}

void GPUHairSim::update_forces(const GLfloat *force_data)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, force_tex);
  glTexSubImage2D(GL_TEXTURE_2D,
                  0,                    //mip level
                  0,                    //starting u coord
                  0,                    //starting v coord
                  force_tex_dim[0],     //width of update rect
                  force_tex_dim[1],     //height of update rect
                  GL_RGBA,              //pixel format
                  GL_FLOAT,
                  force_data);              //pointer to pixel data
}

void GPUHairSim::update_lighting(Float3 sun_pos, Float3 sun_diff, Float3 sun_spec, Float3 ambient, float cam_dist)
{
  sun_pos_xyz = sun_pos;
  sun_diff_rgb = sun_diff;
  sun_spec_rgb = sun_spec;
  ambient_rgb = ambient;
  cam_distance = cam_dist;
}

/*
void GPUHairSim::set_render_shader_names(std::string vs, std::string fs)
{
  render_shader_names[0] = vs;
  render_shader_names[1] = fs;
}


void GPUHairSim::set_simulation_shader_names(std::string vs, std::string fs)
{
  simulation_shader_names[0] = vs;
  simulation_shader_names[1] = fs;
}
*/

void GPUHairSim::simulate(const double game_time, const double dt)
{
  //set the render target to the "current" position texture (0)
  GLint win_viewport[4];
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pos_fbo[0]);
  glGetIntegerv(GL_VIEWPORT, win_viewport);
  glViewport(0, 0, num_hairs, num_segments);

  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  //render the HDR render surface to a full-screen quad
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  sim_mat.render();
  Shader *shader = sim_mat.get_shader();

  //set uniforms + textures
  //dt, k, texel_size, spring_distance
  float texel_size = 1.0f / (float)num_segments;
  float spring_length = hair_height / (float)num_segments;
  glUniform4f(uniform_locations[UNIFORM_SIM_CONSTANTS], (float)dt, spring_constant, texel_size, spring_length);

  //prev pos tex (1)
  glUniform1i(uniform_locations[UNIFORM_SIM_POS_TEX], 0);
  glActiveTexture(GL_TEXTURE0);
  glClientActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, pos_tex[1]);

  //force tex
  glUniform1i(uniform_locations[UNIFORM_SIM_FORCE_TEX], 1);
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, force_tex);

  //uv tex
  glUniform1i(uniform_locations[UNIFORM_SIM_UV_TEX], 2);
  glActiveTexture(GL_TEXTURE2);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, uv_tex);

  //draw calls
  glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(FBOHairVert), (void *)0);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(FBOHairVert), (void *)(sizeof(float) * 3));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo_ibo);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);

  //reset shader
  glUseProgram(0);
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE2);
  glDisable(GL_TEXTURE_2D);

  //release fbo
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glViewport(win_viewport[0], win_viewport[1], win_viewport[2], win_viewport[3]);

  //swap curr / prev textures
  GLuint temp = pos_tex[0];
  pos_tex[0] = pos_tex[1];
  pos_tex[1] = temp;
  temp = pos_fbo[0];
  pos_fbo[0] = pos_fbo[1];
  pos_fbo[1] = temp;
}

void GPUHairSim::render()
{
  glLineWidth(1.0f);

  render_mat.render();

  Shader *shader = render_mat.get_shader();

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  //set up uniforms
  glUniform1i(uniform_locations[UNIFORM_RENDER_HAIR_TEX], 0);
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, pos_tex[0]);

  glUniform1i(uniform_locations[UNIFORM_RENDER_UV_TEX], 1);
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, uv_tex);

  if(color_tex)
  {
    glUniform1i(uniform_locations[UNIFORM_RENDER_COLOR_TEX], 2);
    color_tex->render_gl(GL_TEXTURE2);
  }

  glUniform3f(uniform_locations[UNIFORM_RENDER_SUN_POS], sun_pos_xyz[0], sun_pos_xyz[1], sun_pos_xyz[2]);
  glUniform3f(uniform_locations[UNIFORM_RENDER_DIFF], sun_diff_rgb[0], sun_diff_rgb[1], sun_diff_rgb[2]);
  glUniform3f(uniform_locations[UNIFORM_RENDER_SPEC], sun_spec_rgb[0], sun_spec_rgb[1], sun_spec_rgb[2]);
  glUniform3f(uniform_locations[UNIFORM_RENDER_AMBIENT], ambient_rgb[0], ambient_rgb[1], ambient_rgb[2]);
  glUniform1f(uniform_locations[UNIFORM_RENDER_CAM_DISTANCE], cam_distance);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(HairVert), (void *)0);

  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(3, GL_FLOAT, sizeof(HairVert), (void *)(sizeof(float) * 3));

  glClientActiveTexture(GL_TEXTURE0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(HairVert), (void *)(sizeof(float) * 6));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glDrawElements(GL_LINES, num_indices, GL_UNSIGNED_INT, (void *)0);

  //reset shader
  glUseProgram(0);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE2);
  glDisable(GL_TEXTURE_2D);
}
