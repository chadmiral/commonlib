#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <mxml.h>
#include "skeleton.h"

#define PACKAGE_FILE_VERSION 1

namespace Tool
{
  enum PackageAssetType
  {
    PACKAGE_ASSET_SHADER,
    PACKAGE_ASSET_MATERIAL,
    PACKAGE_ASSET_TEXTURE,
    PACKAGE_ASSET_MESH,
    PACKAGE_ASSET_SKELETON,
    PACKAGE_ASSET_ANIMATION,
    PACKAGE_ASSET_UI_LAYOUT
  };

  class PackageAsset
  {
    friend class PackageBaker;
  protected:
    std::string name;
    std::string fname;
    PackageAssetType type;
    std::vector<std::string> *path;
  public:
    PackageAsset() {}
    PackageAsset(PackageAssetType t) { type = t; }
    ~PackageAsset() {}

    void set_name(const char *n) { name = n; }
    std::string get_name() const { return name; }

    void set_path(std::vector<std::string> *p) { path = p; }

    void set_type(PackageAssetType t) { type = t; }
    PackageAssetType get_type() const { return type; }
  };

  class ShaderPackageAsset : public PackageAsset
  {
  public:
    ShaderPackageAsset() : PackageAsset(PACKAGE_ASSET_SHADER) {}
    ~ShaderPackageAsset() {}

    std::string vs_fname;
    std::string fs_fname;

    std::string vs_source;
    std::string fs_source;

    std::string include_shader(std::string inc_fname);
    void parse_source(std::string source, std::string *dest);
  };

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

  class MeshPackageAsset : public PackageAsset
  {
  public:
    MeshPackageAsset() : PackageAsset(PACKAGE_ASSET_MESH) { num_verts = 0; vertices = NULL; num_indices = 0; indices = NULL; }
    ~MeshPackageAsset() {}

    uint32_t num_verts;
    void *vertices;
    uint32_t num_indices;
    void *indices;
  };

  class MaterialPackageAsset : public PackageAsset
  {
  public:
    MaterialPackageAsset() : PackageAsset(PACKAGE_ASSET_MATERIAL) {}
    ~MaterialPackageAsset() {}

    std::string shader_name;

    uint32_t material_flags;
  };

  class SkeletonPackageAsset : public PackageAsset
  {
  public:
    SkeletonPackageAsset() : PackageAsset(PACKAGE_ASSET_SKELETON) { num_bones = 0; bones = NULL; }
    ~SkeletonPackageAsset() { if (bones) { delete bones; } }

    uint32_t            num_bones;
    Animation::Bone    *bones;
  };

  class AnimationPackageAsset : public PackageAsset
  {
  public:
    AnimationPackageAsset() : PackageAsset(PACKAGE_ASSET_ANIMATION) {  }
    ~AnimationPackageAsset() { }

    Animation::SkeletonAnimation anim;
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

    void read_shader_file(mxml_node_t *shader_node, std::string tabs = "");
    void read_texture_file(mxml_node_t *texture_node, std::string tabs = "");
    void read_mesh_file(mxml_node_t *mesh_node, std::string tabs = "");
    void read_skeleton_file(mxml_node_t *skeleton_node, std::string tabs = "");
    void read_animation_file(mxml_node_t *animation_node, std::string tabs = "");
    void read_ui_layout_file(mxml_node_t *layout_node, std::string tabs = "");

    void write_package(std::string output_fname, std::string tabs = "");

    void write_packlet_header(FILE *fp, PackageAsset *a, std::string tabs = "");

    void write_shader_packlet(FILE *fp, ShaderPackageAsset *s, std::string tabs = "");
    void write_texture_packlet(FILE *fp, TexturePackageAsset *t, std::string tabs = "");
    void write_mesh_packlet(FILE *fp, MeshPackageAsset *m, std::string tabs = "");
    void write_skeleton_packlet(FILE *fp, SkeletonPackageAsset *s, std::string tabs = "");
    void write_animation_packlet(FILE *fp, AnimationPackageAsset *a, std::string tabs = "");
    void write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u, std::string tabs = "");
  public:
    PackageBaker() { file_version = PACKAGE_FILE_VERSION; }
    ~PackageBaker() {}

    void init();
    void bake(mxml_node_t *tree, std::string output_filename, std::string tabs = "");
  };
};