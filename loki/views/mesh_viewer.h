#pragma once

#include "static_mesh.h"
#include "render_surface.h"

class MeshViewer
{
private:
  Graphics::StaticMesh    *_mesh;
  Graphics::Material      *_mesh_mat;
  Graphics::Shader        *_mesh_shader;

  Graphics::ShaderVertexAttrib _mesh_xyz_attrib;

  Graphics::ShaderUniformMatrix4x4 _mesh_projection;
  Graphics::ShaderUniformMatrix4x4 _mesh_modelview;

  Graphics::RenderSurface  _render_target;
  Graphics::Shader         _render_target_shader;

  void render_mesh();

public:
  MeshViewer();
  ~MeshViewer() { deinit(); }

  void init();
  void deinit();

  void render();

  bool visible;
};