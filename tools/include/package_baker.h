#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <mxml.h>
#include "package_asset.h"
#include "shader_baker.h"
#include "material_baker.h"
#include "skeleton_baker.h"
#include "static_mesh_baker.h"
#include "animation_baker.h"

#define PACKAGE_FILE_VERSION 1

namespace Tool
{

  class TexturePackageAsset : public PackageAsset
  {
  public:
    TexturePackageAsset() : PackageAsset(PACKAGE_ASSET_TEXTURE) { tex_data = NULL; tex_data_size = 0; wrap_u = GL_REPEAT; wrap_v = GL_REPEAT; }
    ~TexturePackageAsset() { if (tex_data) { delete tex_data; } }

    uint32_t width;  //texture width
    uint32_t height; //texture height
    uint32_t bpp;    //bytes per pixel
    uint32_t wrap_u; //wrap mode for u axis
    uint32_t wrap_v; //wrap mode for v axis

    void     *tex_data;       //pointer to the actual texture data
    uint32_t tex_data_size;   //size of the texture data
  };

  class UILayoutPackageAsset : public PackageAsset
  {
  public:
    UILayoutPackageAsset() : PackageAsset(PACKAGE_ASSET_UI_LAYOUT) {}
    ~UILayoutPackageAsset() {}

    std::string xml_source;
  };

  class PackageBaker
  {
  private:
    uint32_t file_version;
    std::vector<PackageAsset *> assets;
    std::vector<std::string> asset_path;

    //TODO: shouldn't these be member functions of each package baker class?
    void read_shader_file(mxml_node_t *shader_node, std::string tabs = "");
    void read_material_file(mxml_node_t *material_node, std::string tabs = "", std::ostream &log = std::cout);
    void read_texture_file(mxml_node_t *texture_node, std::string tabs = "");
    void read_mesh_file(mxml_node_t *mesh_node, std::string tabs = "");
    void read_skeleton_file(mxml_node_t *skeleton_node, std::string tabs = "");
    void read_animation_file(mxml_node_t *animation_node, std::string tabs = "");
    void read_ui_layout_file(mxml_node_t *layout_node, std::string tabs = "");

    void write_package(std::string output_fname, std::string tabs = "", std::ostream &log = std::cout);

    void write_packlet_header(FILE *fp, PackageAsset *a, std::ostream &log = std::cout);

    void write_shader_packlet(FILE *fp, ShaderPackageAsset *s, std::string tabs = "", std::ostream &log = std::cout);
    void write_texture_packlet(FILE *fp, TexturePackageAsset *t, std::string tabs = "", std::ostream &log = std::cout);
    void write_material_packlet(FILE *fp, MaterialPackageAsset *m, std::string tabs = "", std::ostream &log = std::cout);
    void write_mesh_packlet(FILE *fp, MeshPackageAsset *m, std::string tabs = "", std::ostream &log = std::cout);
    void write_skeleton_packlet(FILE *fp, SkeletonPackageAsset *s, std::string tabs = "", std::ostream &log = std::cout);
    void write_animation_packlet(FILE *fp, AnimationPackageAsset *a, std::string tabs = "", std::ostream &log = std::cout);
    void write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u, std::string tabs = "", std::ostream &log = std::cout);
  public:
    PackageBaker() { file_version = PACKAGE_FILE_VERSION; }
    ~PackageBaker() {}

    void init();
    void bake(mxml_node_t *tree, std::string output_filename, std::string tabs = "");
  };
};