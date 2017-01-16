#pragma once

#include <imgui.h>
#include <vector>
#include <string>

#include "addons/imguinodegrapheditor/imguinodegrapheditor.h"

#include "material_baker.h"

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
  "GL_ZERO",
  "GL_ZERO",
  "GL_ONE",
  "GL_ONE_MINUS_SRC_ALPHA",
  "GL_SRC_ALPHA_SATURATE",
  "GL_SRC_ALPHA"
};

static const char *BlendModePrettyNames[NUM_BLEND_MODES] =
{
  "No Blending",
  "0",
  "1",
  "(1 - src_alpha)",
  "sat(src_alpha)",
  "src_alpha"
};

class MeshInputNode : public ImGui::Node
{
public:
  MeshInputNode(const ImVec2 &pos) : ImGui::Node()
  {
    strcpy(Name, "Mesh Vertex Data");
    InputsCount = 0;

    OutputsCount = 3;
    strcpy(OutputNames[0], "in_xyz");
    strcpy(OutputNames[1], "in_rgb");
    strcpy(OutputNames[2], "in_uv0");

    Pos = pos;
  }
  ~MeshInputNode() {}

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    if (ImGui::Button("+"))
    {
      strcpy(OutputNames[OutputsCount], "new");
      OutputsCount++;
    }

    return ret;
  }
};

class VertexShaderOutputNode : public ImGui::Node
{
public:
  VertexShaderOutputNode(const ImVec2 &pos) : ImGui::Node()
  {
    Pos = pos;

    strcpy(Name, "VS Output");
    InputsCount = 1;
    strcpy(InputNames[0], "out_xyzw");
    OutputsCount = 0;
  }
  ~VertexShaderOutputNode() {}

  virtual bool render(float node_width) { return ImGui::Node::render(node_width); }
};

class TextureNode : public ImGui::Node
{
public:
  TextureNode(const ImVec2 &pos) : ImGui::Node()
  {
    Pos = pos;
    strcpy(Name, "Texture");
    InputsCount = 0;
    OutputsCount = 1;
    strcpy(OutputNames[0], "rgba");
  }
  ~TextureNode() {}

  virtual bool render(float node_width) { return ImGui::Node::render(node_width); }
};

class UniformNode : public ImGui::Node
{
private:
  int _curr_type;
public:
  UniformNode(const ImVec2 &pos) : ImGui::Node()
  {
    Pos = pos;

    strcpy(Name, "Engine Variable");
    InputsCount = 0;
    OutputsCount = 1;
    strcpy(OutputNames[0], "rgba");
    _curr_type = 0;
  }
  ~UniformNode() {}

  virtual bool render(float node_width)
  {
    bool ret =  ImGui::Node::render(node_width);

    if (ImGui::Combo("type", &_curr_type, "float\0float2\0float3\0float4\0rgb\0rgba\0mat3x3\0mat4x4"))
    {
      switch (_curr_type)
      {
      case 0:
        strcpy(OutputNames[0], "x");
        break;
      case 1:
        strcpy(OutputNames[0], "xy");
        break;
      case 2:
        strcpy(OutputNames[0], "xyz");
        break;
      case 3:
        strcpy(OutputNames[0], "xyzw");
        break;
      case 4:
        strcpy(OutputNames[0], "rgb");
        break;
      case 5:
        strcpy(OutputNames[0], "rgba");
        break;
      case 6:
        strcpy(OutputNames[0], "mat3x3");
        break;
      case 7:
        strcpy(OutputNames[0], "mat4x4");
        break;
      }
    }

    return ret;
  }
};

static const uint32_t NumMathOperations = 5;
static const char *MathOperationNames[] =
{
  "Add",
  "Subtract",
  "Multiply",
  "Divide",
  "Exponent"
};

class MathNode : public ImGui::Node
{
private:
  int _curr_op;
public:
  MathNode(const ImVec2 &pos) : ImGui::Node()
  {
    Pos = pos;
    strcpy(Name, "Math");
    InputsCount = 2;
    strcpy(InputNames[0], "a");
    strcpy(InputNames[1], "b");

    OutputsCount = 1;
    strcpy(OutputNames[0], "rgba");

    _curr_op = 0;
  }
  ~MathNode() {}

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    ImGui::Combo("", &_curr_op, MathOperationNames, NumMathOperations);

    return ret;
  }
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

  ImGui::NodeGraphEditor    _nge;
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
};