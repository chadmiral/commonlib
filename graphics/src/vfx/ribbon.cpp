#include <iostream>
#include "ribbon.h"
#include "perlin.h"

using namespace Graphics;
using namespace Math;
using namespace std;

Ribbon::Ribbon() : Renderable<RibbonVertex>()
{
  num_segments = 20;

  root_scale = 0.7f;
  root_speed = 0.0001f;
  tex_scroll_speed = 1.0f;

  profile_noise_bounds[0] = 0.2f;
  profile_noise_bounds[1] = 0.35f;
  profile_noise_scale = 0.8f;
  profile_noise_speed = 0.0001f;

  theta_bounds[0] = 0.0f;//M_PI - 0.1f;
  theta_bounds[1] = 0.5f;//M_PI;
  phi_bounds[0] = 0.0f;
  phi_bounds[1] = 0.5f;

  profile_a.bell_curve_cerp(0.45f, 0.55f);
}

Ribbon::~Ribbon()
{
  deallocate_buffers();
}

void Ribbon::init()
{
  num_vertices = num_segments * 2;
  num_indices = num_vertices;
  
  gl_usage = GL_DYNAMIC_DRAW;

  Renderable::allocate_buffers();

  for(int i = 0; i < num_segments; i++)
  {
    index_data[2 * i] = 2 * i;
    index_data[2 * i + 1] = 2 * i + 1;

    vertex_data[2 * i].x = vertex_data[2 * i + 1].x = 0.0f;
    vertex_data[2 * i].y = vertex_data[2 * i + 1].y = 0.0f;
    vertex_data[2 * i].z = vertex_data[2 * i + 1].z = 0.0f;

    vertex_data[2 * i].nx = vertex_data[2 * i + 1].nx = 1.0f;
    vertex_data[2 * i].ny = vertex_data[2 * i + 1].ny = 0.0f;
    vertex_data[2 * i].nz = vertex_data[2 * i + 1].nz = 0.0f;

    vertex_data[2 * i].r = vertex_data[2 * i + 1].r = 1.0f;
    vertex_data[2 * i].g = vertex_data[2 * i + 1].g = 1.0f;
    vertex_data[2 * i].b = vertex_data[2 * i + 1].b = 1.0f;

    vertex_data[2 * i].u0 = (float)i / (float)(num_segments - 1);
    vertex_data[2 * i].v0 = 0.0f;

    vertex_data[2 * i + 1].u0 = vertex_data[2 * i].u0;
    vertex_data[2 * i + 1].v0 = 1.0f;
  }

  Renderable::init_buffers();
}

void Ribbon::simulate(const double gt, const double dt)
{
  float ribbon_length = 0.5f; //in radians?
  
  float theta_offset = 0.0f;//(float)M_PI;
  float phi_offset = (float)M_PI * 0.5f;// - ribbon_length * 0.5f * M_PI;
  

  //TODO: use dynamic texture and vertex-shader displacement
  for(int i = 0; i < num_segments; i++)
  {
    float seg_pct = (float)i / (float)num_segments;
    float theta = theta_offset + PerlinNoise::scaled_octave_noise_2d(2.0f, 1.0f, root_scale, theta_bounds[0], theta_bounds[1], (float)gt * root_speed, seg_pct);
    float phi = PerlinNoise::scaled_octave_noise_2d(2.0f, 1.0f, root_scale, phi_bounds[0], phi_bounds[1], (float)gt * root_speed + 137.9532f, seg_pct);
    phi = phi_offset + phi;// + seg_pct * ribbon_length;

    float profile_height = PerlinNoise::scaled_octave_noise_2d(3.0f, 1.0f, profile_noise_scale, profile_noise_bounds[0], profile_noise_bounds[1], (float)gt * profile_noise_speed + 165.132f, seg_pct * profile_noise_scale);
    profile_height *= profile_a.evaluate(seg_pct);

    Float3 root_vert_pos = polar_to_cartesian(theta, phi, 1.05f);
    Float3 top_vert_pos = root_vert_pos + root_vert_pos * profile_height;

    //TODO: actually normal correctly
    Float3 n = root_vert_pos;
    Float3 tmp = n ^ Float3(0.0f, 1.0f, 0.0f);
    n = n ^ tmp;
    n.normalize();

    int i0 = i * 2;
    int i1 = i0 + 1;

    vertex_data[i0].x = root_vert_pos._val[0];
    vertex_data[i0].y = root_vert_pos._val[1];
    vertex_data[i0].z = root_vert_pos._val[2];

    vertex_data[i0].nx = n._val[0];
    vertex_data[i0].ny = n._val[1];
    vertex_data[i0].nz = n._val[2];

    vertex_data[i1].x = top_vert_pos._val[0];
    vertex_data[i1].y = top_vert_pos._val[1];
    vertex_data[i1].z = top_vert_pos._val[2];

    vertex_data[i1].nx = n._val[0];
    vertex_data[i1].ny = n._val[1];
    vertex_data[i1].nz = n._val[2];
  }

  //update the vbo, and release
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RibbonVertex) * num_vertices, &vertex_data[0].x);
}

void Ribbon::render(const double game_time)
{
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  material->render();
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(RibbonVertex), (void *)0);

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(RibbonVertex), (void *)(sizeof(float) * 3));

  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(3, GL_FLOAT, sizeof(RibbonVertex), (void *)(sizeof(float) * 6));
  glClientActiveTexture(GL_TEXTURE0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(RibbonVertex), (void *)(sizeof(float) * 9));


  glDrawElements(geo_mode, num_indices, GL_UNSIGNED_INT, (void *)0);

  material->cleanup();

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glClientActiveTexture(GL_TEXTURE0);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}