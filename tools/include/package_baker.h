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
#include "lens_flare_baker.h"
#include "html_log_stream.h"

#if defined __LOKI__
#define GL_CLAMP 0x2900 //TODO: do this more gracefully
#endif

#define PACKAGE_FILE_VERSION 1

namespace Tool
{
  struct TextureTemplate
  {
    std::string _name;
    std::string _format;
    std::string _wrap_u;
    std::string _wrap_v;
    std::string _wrap_w;
    std::string _fname;
    uint32_t    _slices; // for 3D textures
  };

  class TexturePackageAsset : public PackageAsset
  {
  public:
    TexturePackageAsset() : PackageAsset(PACKAGE_ASSET_TEXTURE),
      tex_data(NULL),
      tex_data_size(0),
      wrap_u(GL_REPEAT),
      wrap_v(GL_REPEAT),
      wrap_w(GL_REPEAT),
      slices(1) {}

    TexturePackageAsset(TextureTemplate &tt) : PackageAsset(PACKAGE_ASSET_TEXTURE),
      tex_data(NULL),
      tex_data_size(0),
      wrap_u(GL_REPEAT),
      wrap_v(GL_REPEAT),
      wrap_w(GL_REPEAT),
      slices(tt._slices)
    {
      name = tt._name;
      fname = tt._fname;

      if (tt._wrap_u.length() > 0 && !stricmp(tt._wrap_u.c_str(), "clamp"))
      {
        wrap_u = GL_CLAMP;
      }
      if (tt._wrap_v.length() > 0 && !stricmp(tt._wrap_v.c_str(), "clamp"))
      {
        wrap_v = GL_CLAMP;
      }
    }
    ~TexturePackageAsset() { if (tex_data) { delete tex_data; } }

    uint32_t width;  //texture width
    uint32_t height; //texture height
    uint32_t depth;  //texture depth

    uint32_t slices; //for 3d textures
    uint32_t bpp;    //bytes per pixel

    uint32_t wrap_u; //wrap mode for u axis
    uint32_t wrap_v; //wrap mode for v axis
    uint32_t wrap_w; //wrap_mode for w axis

    void     *tex_data;       //pointer to the actual texture data
    uint32_t tex_data_size;   //size of the texture data
  };


  struct UILayoutTemplate
  {
    std::string _name;
    std::string _fname;
  };

  class UILayoutPackageAsset : public PackageAsset
  {
  public:
    UILayoutPackageAsset() : PackageAsset(PACKAGE_ASSET_UI_LAYOUT) {}
    UILayoutPackageAsset(UILayoutTemplate &ut) : PackageAsset(PACKAGE_ASSET_UI_LAYOUT)
    {
      name = ut._name;
      fname = ut._fname;
    }
    ~UILayoutPackageAsset() {}

    std::string xml_source;
  };

  struct PackageTemplate
  {
    uint32_t                          _version;

    std::string                       _root_dir;
    std::string                       _path;
    std::string                       _output_file;

    std::vector<ShaderTemplate>       _shaders;
    std::vector<TextureTemplate>      _textures;
    std::vector<MeshTemplate>         _meshes;
    std::vector<SkeletonTemplate>     _skeletons;
    std::vector<AnimationTemplate>    _animations;
    std::vector<MaterialTemplate>     _materials;
    std::vector<UILayoutTemplate>     _ui_layouts;
    std::vector<BasicTemplate>        _lens_flares;
  };

  class PackageBaker
  {
  private:
    uint32_t file_version;
    std::vector<PackageAsset *> assets;
    std::vector<std::string> asset_path;
    HtmlLogStream *_html_logger;

    //TODO: shouldn't these be member functions of each package baker class?
    void parse_basic_xml(mxml_node_t *baic_node, BasicTemplate &bt);
    void parse_shader_xml(mxml_node_t *shader_node, ShaderTemplate &st);
    void parse_material_xml(mxml_node_t *material_node, MaterialTemplate &mt);
    void parse_texture_xml(mxml_node_t *texture_node, TextureTemplate &tt);
    void parse_mesh_xml(mxml_node_t *mesh_node, MeshTemplate &mt);
    void parse_skeleton_xml(mxml_node_t *skeleton_node, SkeletonTemplate &st);
    void parse_animation_xml(mxml_node_t *anim_node, AnimationTemplate &at);
    void parse_ui_layout_xml(mxml_node_t *layout_node, UILayoutTemplate &ut);

    //TODO: shouldn't these be member functions of each package baker class?
    void read_shader_file(ShaderTemplate &st, std::string tabs = "", std::ostream &log = std::cout);
    void read_material_file(MaterialTemplate &mt, std::ostream &log = std::cout);
    void read_texture_file(TextureTemplate &tt, std::ostream &log = std::cout);
    void read_mesh_file(MeshTemplate &mt, std::ostream &log = std::cout);
    void read_skeleton_file(SkeletonTemplate &st, std::ostream &log = std::cout);
    void read_animation_file(AnimationTemplate &at, std::ostream &log = std::cout);
    void read_ui_layout_file(UILayoutTemplate &ut, std::ostream &log = std::cout);
    void read_lens_flare_file(BasicTemplate &bt, std::ostream &log = std::cout);

    void write_package(std::string output_fname, std::ostream &log = std::cout, std::string tabs = "");

    void write_packlet_header(FILE *fp, PackageAsset *a, std::ostream &log = std::cout);

    //TODO: shouldn't these be member functions of each package baker class?
    void write_shader_packlet(FILE *fp, ShaderPackageAsset *s, std::string tabs = "", std::ostream &log = std::cout);
    void write_texture_packlet(FILE *fp, TexturePackageAsset *t, std::string tabs = "", std::ostream &log = std::cout);
    void write_material_packlet(FILE *fp, MaterialPackageAsset *m, std::string tabs = "", std::ostream &log = std::cout);
    void write_mesh_packlet(FILE *fp, MeshPackageAsset *m, std::string tabs = "", std::ostream &log = std::cout);
    void write_skeleton_packlet(FILE *fp, SkeletonPackageAsset *s, std::string tabs = "", std::ostream &log = std::cout);
    void write_animation_packlet(FILE *fp, AnimationPackageAsset *a, std::string tabs = "", std::ostream &log = std::cout);
    void write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u, std::string tabs = "", std::ostream &log = std::cout);
    void write_lens_flare_packlet(FILE *fp, LensFlarePackageAsset *lf, std::string tabs = "", std::ostream &log = std::cout);
  public:
    PackageBaker() { file_version = PACKAGE_FILE_VERSION; }
    ~PackageBaker() {}

    void init() {}
    void parse_xml(mxml_node_t *tree, PackageTemplate &pt, std::ostream &log = std::cout);
    void bake(mxml_node_t *tree, std::string output_filename, PackageTemplate &pt, std::ostream &log = std::cout, std::string tabs = "");
    void set_html_logger(HtmlLogStream *logger) { _html_logger = logger; }
  };
};
