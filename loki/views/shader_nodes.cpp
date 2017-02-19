#include "shader_nodes.h"
#include "material_tool.h"

void FSInputNode::update_links()
{
  MaterialTool *mt = (MaterialTool *)user_ptr;
  OutputsCount = mt->_varying_vars.size();
  for (uint32_t i = 0; i < mt->_varying_vars.size(); i++)
  {
    std::string name = std::to_string(i);
    strcpy(OutputNames[i], name.c_str());
  }
}

bool FSInputNode::render(float node_width)
{
  bool ret = ImGui::Node::render(node_width);
  MaterialTool *mt = (MaterialTool *)user_ptr;

  update_links();

  for (uint32_t i = 0; i < mt->_varying_vars.size(); i++)
  {
    std::stringstream ss;
    ss << i << ": " << mt->_varying_vars[i].second.c_str();
    ImGui::Text(ss.str().c_str());
  }

  if (ImGui::Button("+"))
  {
    std::string var_name = std::string("var_") + std::to_string(OutputsCount);
    GLSLType type = GLSL_VEC3;
    add_var(var_name, type);

    mt->_varying_vars.push_back(std::make_pair(type, var_name));
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Varying variables pass and interpolate data from the vertex to the fragment shader");

  return ret;
}


VertexShaderOutputNode::VertexShaderOutputNode(const ImVec2 &pos) : ShaderNode(pos, SHADER_NODE_VS_OUTPUT)
{
  InputsCount = 0;
  OutputsCount = 0;
}

void VertexShaderOutputNode::update_links()
{
  MaterialTool *mt = (MaterialTool *)user_ptr;
  InputsCount = mt->_varying_vars.size();
  for (uint32_t i = 0; i < mt->_varying_vars.size(); i++)
  {
    std::string name = std::to_string(i);
    strcpy(InputNames[i], name.c_str());
  }
}

bool VertexShaderOutputNode::render(float node_width)
{
  MaterialTool *mt = (MaterialTool *)user_ptr;

  update_links();
  bool ret = ImGui::Node::render(node_width);

  for (uint32_t i = 0; i < mt->_varying_vars.size(); i++)
  {
    std::stringstream ss;
    ss << i << ": " << mt->_varying_vars[i].second.c_str();
    ImGui::Text(ss.str().c_str());
  }

  if (ImGui::Button("+"))
  {
    std::string var_name = std::string("var_") + std::to_string(InputsCount);
    GLSLType type = GLSL_VEC3;

    mt->_varying_vars.push_back(std::make_pair(type, var_name));
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Varying variables pass and interpolate data from the vertex to the fragment shader");

  return ret;
}

void VertexShaderOutputNode::generate_glsl(std::ostream &os, int output_idx) const
{
  MaterialTool *mt = (MaterialTool *)user_ptr;

  for (uint32_t i = 0; i < mt->_varying_vars.size(); i++)
  {
    os << "\t" << mt->_varying_vars[i].second.c_str() << " = ";
    _inputs[i]->generate_glsl(os, _out_connections[i]);
    os << ";" << std::endl;
  }
}