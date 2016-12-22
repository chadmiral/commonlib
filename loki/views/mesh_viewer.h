#pragma once

#include <imgui.h>

#include "static_mesh.h"
#include "render_surface.h"
#include "camera.h"

class MeshViewer
{
private:
  Graphics::Camera         _cam;

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

public:
  MeshViewer();
  ~MeshViewer() { deinit(); }

  void init();
  void deinit();

  void render();

  bool visible;
};