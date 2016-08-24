#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <mxml.h>

#define PACKAGE_FILE_VERSION 1

namespace Tool
{
  enum PackageAssetType
  {
    PACKAGE_ASSET_SHADER,
    PACKAGE_ASSET_TEXTURE,
    PACKAGE_ASSET_MESH,
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
    ShaderPackageAsset() {}
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
    TexturePackageAsset() { tex_data = NULL; wrap_u = GL_REPEAT; wrap_v = GL_REPEAT; }
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
    MeshPackageAsset() {}
    ~MeshPackageAsset() {}

    uint32_t num_verts;
    void *vertices;
    uint32_t num_indices;
    void *indices;
  };

  class MaterialPackageAsset : public PackageAsset
  {
  public:
    MaterialPackageAsset() {}
    ~MaterialPackageAsset() {}

    std::string shader_name;

    uint32_t material_flags;
  };

  class UILayoutPackageAsset : public PackageAsset
  {
  public:
    UILayoutPackageAsset() {}
    ~UILayoutPackageAsset() {}

    std::string xml_source;
  };

  class PackageBaker
  {
  private:
    uint32_t file_version;
    std::vector<PackageAsset *> assets;
    std::vector<std::string> asset_path;

    void read_shader_file(mxml_node_t *shader_node);
    void read_texture_file(mxml_node_t *texture_node);
    void read_ui_layout_file(mxml_node_t *layout_node);

    void write_package(std::string output_fname);
    void write_shader_packlet(FILE *fp, ShaderPackageAsset *s);
    void write_texture_packlet(FILE *fp, TexturePackageAsset *t);
    void write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u);
  public:
    PackageBaker() { file_version = PACKAGE_FILE_VERSION; }
    ~PackageBaker() {}

    void init();
    void bake(mxml_node_t *tree, std::string output_filename);
  };
};