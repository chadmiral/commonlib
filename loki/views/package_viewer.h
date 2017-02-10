#pragma once

#include <imgui.h>
#include <string>
#include "loki_view.h"
#include "package_baker.h"

class PackageViewer : public LokiView
{
private:
  Tool::PackageTemplate       _pt;
  std::string                 _curr_pkg_fname;

  int                         _ui_curr_mesh;
  std::vector<std::string>    _ui_mesh_names;
  int                         _ui_curr_material;
  std::vector<std::string>    _ui_material_names;
  int                         _ui_curr_shader;
  std::vector<std::string>    _ui_shader_names;
  int                         _ui_curr_texture;
  std::vector<std::string>    _ui_texture_names;
  int                         _ui_curr_animation;
  std::vector<std::string>    _ui_animation_names;
  int                         _ui_curr_skeleton;
  std::vector<std::string>    _ui_skeleton_names;
  int                         _ui_curr_ui_layout;
  std::vector<std::string>    _ui_ui_layout_names;

public:
  PackageViewer();
  ~PackageViewer() {}
  virtual void render();

  void load_package(std::string pkg_fname);
  void save_package(std::string pkg_fname);
};