#pragma once

#include <imgui.h>
#include <string>
#include "loki_view.h"
#include "package_baker.h"
#include "package_asset.h"

class PackageViewer : public LokiView
{
private:
  Tool::PackageTemplate       _pt;
  std::string                 _curr_pkg_fname;

  int _ui_curr_selection[Tool::NUM_PACKAGE_ASSET_TYPES];

  std::vector<std::string>    _ui_mesh_names;
  std::vector<std::string>    _ui_material_names;
  std::vector<std::string>    _ui_shader_names;
  std::vector<std::string>    _ui_texture_names;
  std::vector<std::string>    _ui_animation_names;
  std::vector<std::string>    _ui_skeleton_names;
  std::vector<std::string>    _ui_ui_layout_names;

  int                         _curr_new_asset;
public:
  PackageViewer();
  ~PackageViewer() {}
  virtual void render();

  void load_package(std::string pkg_fname);
  void save_package(std::string pkg_fname);
private:
  Tool::PackageAssetType get_curr_asset_type();
  void reset_selection(int32_t mask_id);

  void render_mesh_ui();
  void render_material_ui();
  void render_shader_ui();
  void render_texture_ui();
  void render_animation_ui();
  void render_skeleton_ui();
  void render_ui_layout_ui();

  void add_new_asset();
};