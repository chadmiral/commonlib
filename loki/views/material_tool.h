#pragma once

#include <imgui.h>
#include <vector>
#include <string>

#include "addons/imguinodegrapheditor/imguinodegrapheditor.h"
#include "addons/imguitabwindow/imguitabwindow.h"

#include "material_baker.h"
#include "shader_nodes.h"

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

static const uint32_t Num_uniform_types = 6;
static const char *UniformTypeNames[] =
{
  "float",
  "float2",
  "float3",
  "float4",
  "mat3x3",
  "mat4x4"
};

static const uint32_t Num_culling_modes = 3;
static const char *CullingModeNames[] =
{
  "none",
  "ccw",
  "cw"
};

static const uint32_t Num_render_targets = 3;
static const char *RenderTargetNames[] =
{
  "gbuffer",
  "distortion",
  "depth"
};

static const uint32_t Num_blend_modes = 6;
static const char *BlendModeNames[] =
{
  "GL_ZERO",
  "GL_ZERO",
  "GL_ONE",
  "GL_ONE_MINUS_SRC_ALPHA",
  "GL_SRC_ALPHA_SATURATE",
  "GL_SRC_ALPHA"
};

static const char *BlendModePrettyNames[] =
{
  "No Blending",
  "0",
  "1",
  "(1 - src_alpha)",
  "sat(src_alpha)",
  "src_alpha"
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

  std::string               _last_fname;

  ImGui::TabWindow          _tab_window;
  ImGui::NodeGraphEditor    _nge[2];
public:
  MaterialTool();
  ~MaterialTool() { deinit(); }

  void init() {}
  void deinit() {}

  void render();

  bool visible;
private:
  void open();
  void save_as(std::string fname);

  void add_texture(std::string name);
  void delete_texture(uint32_t idx);
  void add_uniform(std::string name, uint32_t type);
  void delete_uniform(uint32_t idx);
  void add_attrib(std::string name, uint32_t offset, uint32_t stride);
  void delete_attrib(uint32_t idx);

  void generate_glsl(std::ostream &codex = std::cout); //TODO: put this elsewhere (make a ShaderView class?)
};