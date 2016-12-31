#pragma once

#include "tool.h"
#include "package_asset.h"
#include "platform_gl.h"
#include "matrix.h"

namespace Tool
{
  enum TmpUniformType
  {
    TMP_UNIFORM_FLOAT,
    TMP_UNIFORM_FLOAT2,
    TMP_UNIFORM_FLOAT3,
    TMP_UNIFORM_FLOAT4,
    TMP_UNIFORM_MAT4X4,

    NUM_TMP_UNIFORM_TYPES
  };

  struct TmpUniform
  {
    std::string    _name;
    TmpUniformType _type;
    
    /*
    float             _val_float;
    Math::Float2      _val_float2;
    Math::Float3      _val_float3;
    Math::Float4      _val_float4;
    Math::Matrix4x4   _val_mat4x4;
    */
  };

  struct TmpAttrib
  {
    std::string    _name;
    uint32_t       _offset;
    uint32_t       _stride;
  };

  class TmpMaterial
  {
  public:
    TmpMaterial() {}
    ~TmpMaterial() {}

    void write_to_file(FILE *f);
    void read_from_file(FILE *f);

    std::string     _name;
    std::string     _target_buffer;
    std::string     _shader_name;

    uint8_t         _vertex_size;
    bool            _backface_cull;
    GLenum          _winding_order;

    bool            _blending;
    GLenum          _dst_blend_mode;
    GLenum          _src_blend_mode;

    bool            _depth_read;
    bool            _depth_write;
    
    std::vector<std::string> _texture_names;
    std::vector<TmpUniform>  _uniforms;
    std::vector<TmpAttrib>   _attribs;
  };

  class MaterialPackageAsset : public PackageAsset
  {
  public:
    MaterialPackageAsset() : PackageAsset(PACKAGE_ASSET_MATERIAL)
    {
      _file_size = 0;
      _file_data = NULL;
    }
    ~MaterialPackageAsset()
    {
      if (_file_data) { delete _file_data; }
    }

    uint32_t _file_size;
    uint8_t *_file_data;
  };

  class MaterialBaker
  {
  public:
    MaterialBaker() {}
    ~MaterialBaker() {}

    void init() {}
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "", std::ostream &log = std::cout);
  };
};