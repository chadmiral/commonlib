#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include <sstream>

#include "addons/imguinodegrapheditor/imguinodegrapheditor.h"

class MaterialTool;

enum ShaderNodeType
{
  SHADER_NODE_VS_OUTPUT,
  SHADER_NODE_FS_OUTPUT,
  SHADER_NODE_VS_INPUT,
  SHADER_NODE_FS_INPUT,
  SHADER_NODE_TEXTURE2D,
  SHADER_NODE_MATH,
  SHADER_NODE_SPLIT,
  SHADER_NODE_JOIN,
  SHADER_NODE_CONSTANT_VARIABLE,
  SHADER_NODE_UNIFORM_VARIABLE,
  SHADER_NODE_GLSL,
  SHADER_NODE_TEXT,

  NUM_SHADER_NODE_TYPES
};

static const char *Shader_node_type_names[] =
{
  "Shader Bridge (VS)",
  "Fragment Shader Out",
  "Vertex Input",
  "Shader Bridge (FS)",
  "Texture 2D",
  "Math",
  "Split",
  "Join",
  "Constant Variable",
  "Engine Variable",
  "GLSL",
  "Note"
};

enum GLSLType
{
  GLSL_FLOAT,
  GLSL_VEC2,
  GLSL_VEC3,
  GLSL_VEC4,
  GLSL_MAT3,
  GLSL_MAT4,

  NUM_GLSL_TYPES
};

static const char *GLSL_type_names[] =
{
  "float",
  "vec2",
  "vec3",
  "vec4",
  "mat3",
  "mat4"
};

class ShaderNode : public ImGui::Node
{
protected:
  std::vector<ShaderNode *> _inputs;
  std::vector<uint32_t>     _out_connections;

  std::string               _prefetch_name;
  uint32_t                  _prefetch_counter;
public:
  ShaderNode(const ImVec2 &pos, ShaderNodeType t) : ImGui::Node()
  {
    Pos = pos;
    strcpy(Name, Shader_node_type_names[t]);
    isOpen = true;
    typeID = t;
    _prefetch_counter = 0;
  }

  ~ShaderNode() {}

  void prep_for_code_generation()
  {
    _prefetch_counter = 0;
  }

  void set_prefetch_name(std::string n)
  {
    _prefetch_name = n;
  }
  std::string get_prefetch_name() { return _prefetch_name; }

  virtual bool render(float node_width) = 0;
  virtual void generate_glsl(std::ostream &os, int output_idx) const = 0;
  virtual void generate_prefetch_declarations(std::string whitespace, std::ostream &os, int output_idx)
  {
    for (uint32_t i = 0; i < _inputs.size(); i++)
    {
      _inputs[i]->generate_prefetch_declarations(whitespace, os, _out_connections[i]);
    }
  }

  void add_link(ImGui::NodeLink l)
  {
    if (_inputs.size() <= l.OutputSlot)
    {
      _inputs.resize(l.OutputSlot + 1);
      _out_connections.resize(l.OutputSlot + 1);
    }
    _inputs[l.OutputSlot] = (ShaderNode *)l.InputNode;
    _out_connections[l.OutputSlot] = l.InputSlot;
  }

  void remove_link(ImGui::NodeLink l)
  {
    _inputs[l.OutputSlot] = NULL;
    _out_connections[l.OutputSlot] = -1;
  }
};

class MeshInputNode : public ShaderNode
{
private:
  std::vector<std::string> _var_names;
public:
  MeshInputNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_VS_INPUT)
  {
    InputsCount = 0;
    OutputsCount = 0;
  }
  ~MeshInputNode() {}

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    for (uint32_t i = 0; i < _var_names.size(); i++)
    {
      std::stringstream ss;
      ss << i << ": " << _var_names[i].c_str();
      ImGui::Text(ss.str().c_str());
    }

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << _var_names[output_idx].c_str();
  }

  virtual void generate_prefetch_declarations(std::string whitespace, std::ostream &os, int output_idx) {}

  void variable_declaration(std::ostream &os, uint32_t idx)
  {
    std::string var_type("vec3");
    os << "in " << var_type.c_str() << " " << _var_names[idx].c_str() << ";" << std::endl;
  }

  void add_var(std::string name, GLSLType type)
  {
    _var_names.push_back(name);
    strcpy(OutputNames[OutputsCount], "");
    std::string label = std::to_string(OutputsCount);
    strcpy(OutputNames[OutputsCount++], label.c_str());
  }
};

class FSInputNode : public ShaderNode
{
private:
  std::vector<std::string> _var_names;
public:
  FSInputNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_FS_INPUT)
  {
    InputsCount = 0;
    OutputsCount = 0;
  }
  ~FSInputNode() {}

  void update_links();

  virtual bool render(float node_width);

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << _var_names[output_idx].c_str();
  }

  virtual void generate_prefetch_declarations(std::string whitespace, std::ostream &os, int output_idx) {}

  void variable_declaration(std::ostream &os, uint32_t idx)
  {
    std::string var_type("vec3");
    os << "in " << var_type.c_str() << " " << _var_names[idx].c_str() << ";" << std::endl;
  }

  void add_var(std::string name, GLSLType type)
  {
    _var_names.push_back(name);
    strcpy(OutputNames[OutputsCount], "");
    std::string label = std::to_string(OutputsCount);
    strcpy(OutputNames[OutputsCount++], label.c_str());
  }
};

class VertexShaderOutputNode : public ShaderNode
{
public:
  VertexShaderOutputNode(const ImVec2 &pos);
  ~VertexShaderOutputNode() {}

  virtual bool render(float node_width);
  void update_links();
  virtual void generate_glsl(std::ostream &os, int output_idx) const;
};

class FragmentShaderOutputNode : public ShaderNode
{
public:
  FragmentShaderOutputNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_FS_OUTPUT)
  {
    InputsCount = 1;
    strcpy(InputNames[0], "rgba");
    OutputsCount = 0;
  }
  ~FragmentShaderOutputNode() {}

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << "<IMPLEMENT_ME>" << std::endl;
  }

  virtual void generate_prefetch_declarations(std::string whitespace, std::ostream &os, int output_idx)
  {
    os << "<PREFETCH: IMPLEMENT_ME>" << std::endl;
  }
};

class TextureNode : public ShaderNode
{
public:
  TextureNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_TEXTURE2D)
  {
    InputsCount = 1;
    strcpy(InputNames[0], "uv");
    OutputsCount = 1;
    strcpy(OutputNames[0], "rgba");
  }
  ~TextureNode() {}

  virtual bool render(float node_width) { return ImGui::Node::render(node_width); }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << _prefetch_name;
  }

  virtual void generate_prefetch_declarations(std::string whitespace, std::ostream &os, int output_idx)
  {
    if (_prefetch_counter == 0)
    {
      _inputs[0]->generate_prefetch_declarations(whitespace, os, _out_connections[0]);
      os << whitespace << "vec4 " << _prefetch_name << " = texture2D(" << Name << ", ";
      _inputs[0]->generate_glsl(os, _out_connections[0]);
      os << ");" << std::endl;

      _prefetch_counter++;
    }
  }

  void generate_prefetch_glsl(std::ostream &os, int output_idx) const
  {
    assert(false);
    //os << "texture2D(" << Name << ", ";
    //_inputs[0]->generate_glsl(os, _out_connections[0]);
    //os << ")";
  }
};

class TextNode : public ShaderNode
{
private:
  char      *_buffer;
  uint32_t   _buffer_size;
  bool       _is_code;
  int        _curr_return_type;
public:
  TextNode(const ImVec2 &pos, bool code) : ShaderNode(pos, code ? SHADER_NODE_GLSL : SHADER_NODE_TEXT)
  {
    InputsCount = 0;
    OutputsCount = 0;

    _is_code = code;

    _buffer_size = 4096;
    _buffer = new char[_buffer_size];
    _buffer[0] = '\0';

    _curr_return_type = 0;

    strcpy(Name, "glsl_function");

    if (code)
    {
      OutputsCount = 1;
      strcpy(OutputNames[0], "out");
      strcpy(_buffer, "//example function\nfloat glsl_function(vec3 a, vec3 b)\n{\n\treturn dot(a, b);\n}");

      InputsCount = 2;
      strcpy(InputNames[0], "a");
      strcpy(InputNames[1], "b");
    }
    else
    {
      strcpy(_buffer, "Useful note");
    }
  }
  ~TextNode() { delete _buffer; }

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    if (ImGui::InputTextMultiline("", _buffer, _buffer_size, ImVec2(300, 100), ImGuiInputTextFlags_AllowTabInput))
    {
      if (strlen(_buffer) > _buffer_size - 5)
      {
        _buffer_size = _buffer_size * 2;
        char *tmp = new char[_buffer_size];
        strcpy(tmp, _buffer);
        delete _buffer;
        _buffer = tmp;
      }
    }

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << _prefetch_name;
  }

  virtual void generate_prefetch_declarations(std::string whitespace, std::ostream &os, int output_idx)
  {
    //backtrace
    for (uint32_t i = 0; i < _inputs.size(); i++)
    {
      _inputs[i]->generate_prefetch_declarations(whitespace, os, _out_connections[i]);
    }

    if (_prefetch_counter == 0)
    {
      std::string type_name = "vec4"; //TODO
      os << whitespace << type_name << " " << _prefetch_name << " = " << Name << "(";
      for (uint32_t i = 0; i < _inputs.size(); i++)
      {
        _inputs[i]->generate_glsl(os, _out_connections[i]);
        if (i != _inputs.size() - 1) { os << ", "; }
      }
      os << ");" << std::endl;

      _prefetch_counter++;
    }
  }

  void generate_prefetch_glsl(std::ostream &os, int output_idx)
  {
    assert(false);
    /*
    os << Name << "("; //TODO
    for (uint32_t i = 0; i < _inputs.size(); i++)
    {
      _inputs[i]->generate_glsl(os, _out_connections[i]);
      if (i != _inputs.size() - 1) { os << ", "; }
    }
    os << ")";
    */
  }

  void function_declaration(std::ostream &os) const
  {
    os << _buffer;
  }

  GLSLType get_return_type() {}
  std::string get_function_name() {}
  void get_function_parameters(std::vector<GLSLType> &types, std::vector<std::string> &names)
  {

  }
};

class ConstantNode : public ShaderNode
{
private:
  int _curr_type;
  float _val[16];
public:
  ConstantNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_CONSTANT_VARIABLE)
  {
    InputsCount = 0;
    OutputsCount = 1;
    strcpy(OutputNames[0], "");
    _curr_type = 0;

    strcpy(Name, "const_var");
    memset(_val, 0, sizeof(float) * 16);
  }
  ~ConstantNode() {}

  std::string get_var_type_name() const
  {
    return GLSL_type_names[_curr_type];
  }

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);


    int32_t item_width = -1;
    switch (_curr_type)
    {
    case GLSL_VEC3:
    case GLSL_MAT3:
      item_width = 200;
      break;
    case GLSL_VEC4:
    case GLSL_MAT4:
      item_width = 400;
      break;
    }
  
    if (item_width > 0)
    {
      ImGui::PushItemWidth(item_width);
    }

    ImGui::InputText("", Name, 32);
    ImGui::Combo("", &_curr_type, GLSL_type_names, NUM_GLSL_TYPES);

    uint32_t ui_id = 0;
    switch (_curr_type)
    {
    case GLSL_FLOAT:
      ImGui::PushID(ui_id++);
      ImGui::InputFloat("", _val);
      ImGui::PopID();
      break;
    case GLSL_VEC2:
      ImGui::PushID(ui_id++);
      ImGui::InputFloat2("", _val);
      ImGui::PopID();
      break;
    case GLSL_VEC3:
      ImGui::PushID(ui_id++);
      ImGui::InputFloat3("", _val);
      ImGui::PopID();
      break;
    case GLSL_VEC4:
      ImGui::PushID(ui_id++);
      ImGui::InputFloat4("", _val);
      ImGui::PopID();
      break;
    case GLSL_MAT3:
      ImGui::PushID(ui_id++);
      ImGui::InputFloat3("", _val);
      ImGui::PopID();
      ImGui::PushID(ui_id++);
      ImGui::InputFloat3("", &(_val[3]));
      ImGui::PopID();
      ImGui::PushID(ui_id++);
      ImGui::InputFloat3("", &(_val[6]));
      ImGui::PopID();

      break;
    case GLSL_MAT4:
      for (uint32_t i = 0; i < 4; i++)
      {
        ImGui::PushID(ui_id++);
        ImGui::InputFloat4("", &_val[i * 4]);
        ImGui::PopID();
      }
      break;
    }

    if (item_width > 0)
    {
      ImGui::PopItemWidth();
    }

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << Name;
  }

  std::string get_value_string()
  {
    std::stringstream ss;
    switch (_curr_type)
    {
    case GLSL_FLOAT:
      ss << _val[0];
      break;
    case GLSL_VEC2:
      ss << "vec2(" << _val[0] << ", " << _val[1] << ")";
      break;
    case GLSL_VEC3:
      ss << "vec3(" << _val[0] << ", " << _val[1] << ", " << _val[2] << ")";
      break;
    case GLSL_VEC4:
      ss << "vec4(" << _val[0] << ", " << _val[1] << ", " << _val[2] << ", " << _val[3] << ")";
      break;
    case GLSL_MAT3:
      ss << "mat3(";
      for (uint32_t i = 0; i < 3; i++)
      {
        ss << "vec3(" << _val[3 * i + 0] << ", " << _val[3 * i + 1] << ", " << _val[3 * i + 2] << ")";
        if (i < 2) { ss << ", "; }
      }
      ss << ")";
      break;
    case GLSL_MAT4:
      ss << "mat4(";
      for (uint32_t i = 0; i < 4; i++)
      {
        ss << "vec4(" << _val[4 * i + 0] << ", " << _val[4 * i + 1] << ", " << _val[4 * i + 2] << ", " <<_val[4 * i + 3] << ")";
        if (i < 3) { ss << ", "; }
      }
      ss << ")";
      break;
    }
    return ss.str();
  }
};

class UniformNode : public ShaderNode
{
private:
  int _curr_type;
public:
  UniformNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_UNIFORM_VARIABLE)
  {
    InputsCount = 0;
    OutputsCount = 1;
    strcpy(OutputNames[0], "");
    _curr_type = 0;

    strcpy(Name, "in_engine");
  }
  ~UniformNode() {}

  std::string get_var_type_name() const
  {
    return GLSL_type_names[_curr_type];
  }

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    ImGui::InputText("", Name, 32);

    if (ImGui::Combo("", &_curr_type, GLSL_type_names, NUM_GLSL_TYPES))
    {
      /*switch (_curr_type)
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
      }*/
    }

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << Name;
  }
};

enum MathNodeOperations
{
  MATH_NODE_ADD,
  MATH_NODE_SUBTRACT,
  MATH_NODE_MULTIPLY,
  MATH_NODE_DIVIDE,

  MATH_NODE_POW,

  MATH_NODE_SIN,
  MATH_NODE_COS,

  MATH_NODE_DOT,
  MATH_NODE_CROSS,
  
  MATH_NODE_LERP,
  MATH_NODE_CLAMP,
  MATH_NODE_MIN,
  MATH_NODE_MAX,

  NUM_MATH_NODE_OPS
};
static const char *MathOperationNames[] =
{
  "(a + b)",
  "(a - b)",
  "(a * b)",
  "(a / b)",

  "pow(a, b)",

  "sin(a)",
  "cos(a)",

  "dot(a, b)",
  "cross(a, b)",

  "lerp(a, b, m)",
  "clamp(a, l, h)",
  "min(a, b)",
  "max(a, b)"
};

class MathNode : public ShaderNode
{
private:
  int _curr_op;
public:
  MathNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_MATH)
  {
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

    ImGui::PushItemWidth(130);
    if (ImGui::Combo("", &_curr_op, MathOperationNames, NUM_MATH_NODE_OPS))
    {
      InputsCount = 2;
      strcpy(InputNames[0], "a");
      strcpy(InputNames[1], "b");

      switch (_curr_op)
      {
        case MATH_NODE_SIN:
        case MATH_NODE_COS:
          InputsCount = 1;
          break;
        case MATH_NODE_ADD:
        case MATH_NODE_SUBTRACT:
        case MATH_NODE_MULTIPLY:
        case MATH_NODE_DIVIDE:
        case MATH_NODE_POW:
        case MATH_NODE_MIN:
        case MATH_NODE_MAX:
          InputsCount = 2;
          break;
        case MATH_NODE_LERP:
          InputsCount = 3;
          strcpy(InputNames[2], "m");
          break;
        case MATH_NODE_CLAMP:
          InputsCount = 3;
          strcpy(InputNames[1], "l");
          strcpy(InputNames[2], "h");
      }
    }
    ImGui::PopItemWidth();

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    if (_curr_op == MATH_NODE_POW)
    {
      gen_func_call(os, "pow", 2);
      return;
    }

    if (_curr_op == MATH_NODE_SIN)
    {
      gen_func_call(os, "sin", 1);
      return;
    }

    if (_curr_op == MATH_NODE_COS)
    {
      gen_func_call(os, "cos", 1);
      return;
    }

    if (_curr_op == MATH_NODE_DOT)
    {
      gen_func_call(os, "dot", 2);
      return;
    }

    if (_curr_op == MATH_NODE_CROSS)
    {
      gen_func_call(os, "cross", 2);
      return;
    }

    if (_curr_op == MATH_NODE_LERP)
    {
      gen_func_call(os, "mix", 3);
      return;
    }

    if (_curr_op == MATH_NODE_CLAMP)
    {
      gen_func_call(os, "clamp", 3);
      return;
    }

    if (_curr_op == MATH_NODE_MIN)
    {
      gen_func_call(os, "min", 2);
      return;
    }

    if (_curr_op == MATH_NODE_MAX)
    {
      gen_func_call(os, "max", 2);
      return;
    }

    os << "(";
    _inputs[0]->generate_glsl(os, _out_connections[0]);
    switch (_curr_op)
    {
    case MATH_NODE_ADD:
      os << " + ";
      break;
    case MATH_NODE_SUBTRACT:
      os << " - ";
      break;
    case MATH_NODE_MULTIPLY:
      os << " * ";
      break;
    case MATH_NODE_DIVIDE:
      os << " / ";
      break;
    }
    
    _inputs[1]->generate_glsl(os, _out_connections[1]);
    os << ")";
  }

  private:
    void gen_func_call(std::ostream &os, std::string func_name, uint32_t num_params) const
    {
      os << func_name.c_str() << "(";
      for (uint32_t i = 0; i < num_params; i++)
      {
        _inputs[i]->generate_glsl(os, _out_connections[i]);
        if (i < num_params - 1) { os << ", "; }
      }
      os << ")";
    }
};

class SplitNode : public ShaderNode
{
private:
  int _curr_type;
public:
  SplitNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_SPLIT)
  {
    _curr_type = 2;

    InputsCount = 1;
    strcpy(InputNames[0], "in");
    OutputsCount = 4;
    strcpy(OutputNames[0], "x");
    strcpy(OutputNames[1], "y");
    strcpy(OutputNames[2], "z");
    strcpy(OutputNames[3], "w");
  }

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    if (ImGui::Combo("", &_curr_type, "vec2\0vec3\0vec4"))
    {
      OutputsCount = _curr_type + 2;
    }

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    char *swizzle_names[] = { ".x", ".y", ".z", ".w" };
    _inputs[0]->generate_glsl(os, _out_connections[0]);
    os << swizzle_names[output_idx];
  }
};

class JoinNode : public ShaderNode
{
private:
  int _curr_type;
public:
  JoinNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_JOIN)
  {
    _curr_type = 2;

    OutputsCount = 1;
    strcpy(OutputNames[0], "out");
    InputsCount = 4;
    strcpy(InputNames[0], "x");
    strcpy(InputNames[1], "y");
    strcpy(InputNames[2], "z");
    strcpy(InputNames[3], "w");
  }

  virtual bool render(float node_width)
  {
    bool ret = ImGui::Node::render(node_width);

    if (ImGui::Combo("", &_curr_type, "vec2\0vec3\0vec4"))
    {
      InputsCount = _curr_type + 2;
    }

    return ret;
  }

  virtual void generate_glsl(std::ostream &os, int output_idx) const
  {
    os << "vec" << InputsCount << "(";
    for (int i = 0; i < InputsCount; i++)
    {
      _inputs[i]->generate_glsl(os, _out_connections[i]);
      //os << "." << component_name[i];
      if (i < _inputs.size() - 1)
      {
        os << ", ";
      }
    }
    os << ")";
  }
};