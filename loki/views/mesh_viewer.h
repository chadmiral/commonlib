#pragma once

#include <imgui.h>

#include "static_mesh.h"
#include "render_surface.h"
#include "camera.h"
#include "string.h"

class MeshViewer
{
private:
  Graphics::Camera         _cam;

  std::string              _data_root; //TODO: app level variable
  std::string              _mesh_fname;
  Graphics::StaticMesh    *_mesh;
  Graphics::Material      *_mesh_mat;
  Graphics::Shader        *_mesh_shader;

  Graphics::ShaderVertexAttrib _mesh_xyz_attrib;

  Graphics::ShaderUniformMatrix4x4 _mesh_projection;
  Graphics::ShaderUniformMatrix4x4 _mesh_modelview;

  Graphics::RenderSurface  _render_target;
  Graphics::Shader         _render_target_shader;

  ImVec2                   _view_size;

  void load_mesh();
  void render_mesh();

  struct TestVertex
  {
    float pos[3];
  };

  //test stuff DELETE
  uint32_t                _num_vertices;
  TestVertex             *_vertices;

  uint32_t                _num_indices;
  uint32_t               *_indices;

  GLuint                  _vbo;
  GLuint                  _ibo;

public:
  MeshViewer();
  ~MeshViewer() { deinit(); }

  void init();
  void deinit();

  void render();

  bool visible;
};