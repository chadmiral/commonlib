#pragma once

#include <imgui.h>
#include <vector>
#include <string>

struct UniformUI
{
  std::string    _name;
  uint32_t       _type;
};

struct AttributeUI
{
  std::string    _name;
  uint32_t       _offset;
  uint32_t       _stride;
};

#define NUM_UNIFORM_TYPES 6
static const char *UniformTypeNames[NUM_UNIFORM_TYPES] =
{
  "float",
  "float2",
  "float3",
  "float4",
  "mat3x3",
  "mat4x4"
};

#define NUM_CULLING_MODES 3
static const char *CullingModeNames[NUM_CULLING_MODES] =
{
  "none",
  "ccw",
  "cw"
};

#define NUM_RENDER_TARGETS 3
static const char *RenderTargetNames[NUM_RENDER_TARGETS] =
{
  "gbuffer",
  "distortion",
  "depth"
};

#define NUM_BLEND_MODES 6
static const char *BlendModeNames[NUM_BLEND_MODES] =
{
  "NO BLENDING",
  "GL_ZERO",
  "GL_ONE",
  "GL_ONE_MINUS_SRC_ALPHA",
  "GL_SRC_ALPHA_SATURATE",
  "GL_SRC_ALPHA"
};

class MaterialTool
{
private:
  char _material_name[128];
  char _shader_name[128];
  char _vertex_size[64];
  int  _target_buffer;

  int  _blend_src_mode;
  int  _blend_dst_mode;

  int  _culling_mode;

  bool _depth_read;
  bool _depth_write;
  
  std::vector<std::string>  _texture_names;
  std::vector<std::string>  _ui_texture_names;
  int                       _ui_curr_texture;
  
  
  std::vector<UniformUI>    _uniforms;
  std::vector<std::string>  _ui_uniform_names;
  int                       _ui_uniform_type;
  int                       _ui_curr_uniform;

  std::vector<AttributeUI>  _attribs;
  std::vector<std::string>  _ui_attrib_names;
  int                       _ui_attrib_type;
  int                       _ui_curr_attrib;

public:
  MaterialTool();
  ~MaterialTool() { deinit(); }

  void init() {}
  void deinit() {}

  void render();

  bool visible;
private:
  void save_as();

  void add_texture(std::string name);
  void delete_texture(uint32_t idx);
  void add_uniform(std::string name, uint32_t type);
  void delete_uniform(uint32_t idx);
  void add_attrib(std::string name, uint32_t offset, uint32_t stride);
  void delete_attrib(uint32_t idx);
};