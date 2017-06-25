#include <vector>
#include <string>
#include <fstream>
#include "platform.h"
#include "material_tool.h"
#include "ui_common.h"

using namespace std;

static void *Node_usr_ptr = NULL;

void tab_content_provider(ImGui::TabWindow::TabLabel *tab, ImGui::TabWindow& parent, void* userPtr) {
  ImGui::Spacing(); ImGui::Separator();
  if (tab)
  {
    ImGui::NodeGraphEditor *nge = (ImGui::NodeGraphEditor *)tab->userPtr;
    nge->render();
  }
  else
  {
    assert(false);
  }
  ImGui::Separator(); ImGui::Spacing();
}


void link_callback(const ImGui::NodeLink& link, ImGui::NodeGraphEditor::LinkState state, ImGui::NodeGraphEditor& editor)
{
  ShaderNode *in = (ShaderNode *)link.InputNode;
  ShaderNode *out = (ShaderNode *)link.OutputNode;
  if (state == ImGui::NodeGraphEditor::LS_ADDED)
  {
    out->add_link(link);
  }
  else if (state == ImGui::NodeGraphEditor::LS_DELETED)
  {
    out->remove_link(link);
  }
}

void node_callback(ImGui::Node*& node, ImGui::NodeGraphEditor::NodeState state, ImGui::NodeGraphEditor& editor)
{
  node->user_ptr = Node_usr_ptr;
}

ImGui::Node* node_factory_delegate(int nodeType, const ImVec2& pos)
{
  ImGui::Node *ret_node = NULL;
  switch (nodeType)
  {
  case SHADER_NODE_VS_OUTPUT:
    ret_node = new VertexShaderOutputNode(pos);
    break;
  case SHADER_NODE_FS_OUTPUT:
    ret_node = new FragmentShaderOutputNode(pos);
    break;
  case SHADER_NODE_VS_INPUT:
    ret_node = new MeshInputNode(pos);
    break;
  case SHADER_NODE_FS_INPUT:
    ret_node = new FSInputNode(pos);
    break;
  case SHADER_NODE_TEXTURE2D:
    ret_node = new TextureNode(pos);
    break;
  case SHADER_NODE_MATH:
    ret_node = new MathNode(pos);
    break;
  case SHADER_NODE_SPLIT:
    ret_node = new SplitNode(pos);
    break;
  case SHADER_NODE_JOIN:
    ret_node = new JoinNode(pos);
    break;
  case SHADER_NODE_CONSTANT_VARIABLE:
    ret_node = new ConstantNode(pos);
    break;
  case SHADER_NODE_UNIFORM_VARIABLE:
    ret_node = new UniformNode(pos);
    break;
  case SHADER_NODE_GLSL:
    ret_node = new TextNode(pos, true);
    break;
  case SHADER_NODE_TEXT:
    ret_node = new TextNode(pos, false);
    break;
  }
  return ret_node;
}

MaterialTool::MaterialTool()
{

  _varying_vars.push_back(std::make_pair(GLSL_VEC4, "out_xyzw"));

  Node_usr_ptr = this;
  visible = true;

  strcpy(_material_name, "Material 01");
  strcpy(_shader_name, "Shader 01");
  strcpy(_vertex_size, "20");
  _target_buffer = 0;

  _blend_src_mode = 5;
  _blend_dst_mode = 3;

  _culling_mode = 1;

  _ui_curr_texture = 0;

  _ui_uniform_type = 0;
  _ui_curr_uniform = 0;

  _ui_attrib_type = 0;
  _ui_curr_attrib = 0;

  _depth_read = true;
  _depth_write = true;

  //add defaults
  add_uniform("game_time", 0);
  add_uniform("proj_mat", 5);
  add_uniform("modelview_mat", 5);
  add_uniform("normal_mat", 5);

  add_attrib("in_xyz", 0, 3);
  add_attrib("in_normal", 3, 3);
  add_attrib("in_rgb", 6, 3);
  add_attrib("in_uv0", 9, 2);

  for (uint32_t i = 0; i < 2; i++)
  {
    _nge[i].show_left_pane = false;
    _nge[i].registerNodeTypes(Shader_node_type_names, NUM_SHADER_NODE_TYPES, node_factory_delegate);
    _nge[i].setLinkCallback(link_callback);
    _nge[i].setNodeCallback(node_callback);
  }

  /*
  ImGui::NodeGraphEditor::Style style = _nge.GetStyle();
  style.node_slots_radius = 20.0f;
  style.grid_size = 128.0f;
  style.color_link = ImColor(0, 0, 255);
  */

  _tab_window.SetWindowContentDrawerCallback(tab_content_provider, NULL);

  //vertex shader
  _nge[0].addNode(SHADER_NODE_VS_OUTPUT, ImVec2(600, 300));
  MeshInputNode *vs_input_node = (MeshInputNode *)_nge[0].addNode(SHADER_NODE_VS_INPUT, ImVec2(150, 300));

  //fragment shader
  _nge[1].addNode(SHADER_NODE_FS_OUTPUT, ImVec2(600, 300));
  FSInputNode *fs_input_node = (FSInputNode *)_nge[1].addNode(SHADER_NODE_FS_INPUT, ImVec2(150, 300));
  
  for (uint32_t i = 0; i < _attribs.size(); i++)
  {
    std::string var_name = _attribs[i]._name;
    GLSLType type = GLSL_VEC3;
    vs_input_node->add_var(var_name, type);
  }
}

void MaterialTool::render()
{
  if (visible)
  {
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Material Editor", &visible, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Open...", NULL, false))
        {
          open();
        }
        if (ImGui::MenuItem("Save", NULL, false))
        {
          save_as(_last_fname);
        }
        if (ImGui::MenuItem("Save As...", NULL, false))
        {
          save_as("");
        }
        if (ImGui::MenuItem("Generate GLSL", NULL, false))
        {
          generate_glsl(0);
          generate_glsl(1);
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    static uint32_t mat_group = 0;
    ImGui::BeginChild(mat_group, ImVec2(350, 0), true);
    {
      ImGui::PushItemWidth(160);
      ImGui::InputText("Material Name", _material_name, 128);
      ImGui::InputText("Vertex Size (Bytes)", _vertex_size, 64, ImGuiInputTextFlags_CharsDecimal);
      ImGui::Combo("Target Buffer", &_target_buffer, RenderTargetNames, Num_render_targets);

      ImGui::Combo("Alpha Blend Src", &_blend_src_mode, BlendModePrettyNames, Num_blend_modes);
      ImGui::Combo("Alpha Blend Dst", &_blend_dst_mode, BlendModePrettyNames, Num_blend_modes);

      ImGui::Combo("Backface Culling", &_culling_mode, CullingModeNames, Num_culling_modes);

      ImGui::InputText("Shader Name", _shader_name, 128);
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(ImGui::GetWindowSize().x);

      //
      // Textures
      //
      static uint32_t tex_group = 1;
      ImGui::BeginChild(tex_group, ImVec2(0, 160), true);

      ImGui::Text("Textures");

      static char new_tex_name[128] = "<New Tex Name>";
      ImGui::PushItemWidth(ImGui::GetWindowSize().x - 60);
      ImGui::InputText("", new_tex_name, 128);
      ImGui::SameLine();

      if (ImGui::Button("+"))
      {
        add_texture(new_tex_name);
      }
      ImGui::SameLine();
      if (ImGui::Button("-"))
      {
        delete_texture(_ui_curr_texture);
      }
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(ImGui::GetWindowSize().x);
      ImGui::ListBox("", &_ui_curr_texture, _ui_texture_names, _ui_texture_names.size(), 5);
      ImGui::PopItemWidth();
      ImGui::EndChild();

      //
      // Uniform Variables
      //
      static uint32_t uniform_group = 2;
      ImGui::BeginChild(uniform_group, ImVec2(0, 160), true);

      ImGui::Text("Uniform Variables");

      static char new_uniform_name[128] = "<New Uniform Name>";
      ImGui::PushItemWidth(ImGui::GetWindowSize().x - 150);
      ImGui::InputText("", new_uniform_name, 128);
      ImGui::SameLine();

      ImGui::PushItemWidth(80);
      ImGui::Combo("", &_ui_uniform_type, UniformTypeNames, Num_uniform_types);
      ImGui::PopItemWidth();
      ImGui::SameLine();

      if (ImGui::Button("+"))
      {
        add_uniform(new_uniform_name, _ui_uniform_type);
      }
      ImGui::SameLine();
      if (ImGui::Button("-"))
      {
        delete_uniform(_ui_curr_uniform);
      }
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(ImGui::GetWindowSize().x);
      ImGui::ListBox("", &_ui_curr_uniform, _ui_uniform_names, _ui_uniform_names.size(), 5);
      ImGui::PopItemWidth();
      ImGui::EndChild();


      //
      // Vertex Attributes
      //
      static uint32_t attrib_group = 3;
      ImGui::BeginChild(attrib_group, ImVec2(0, 180), true);

      ImGui::Text("Vertex Attributes");

      static char new_attrib_name[128] = "<New Attribute Name>";
      ImGui::PushItemWidth(ImGui::GetWindowSize().x);
      ImGui::InputText("", new_attrib_name, 128);
      //ImGui::SameLine();

      ImGui::PushItemWidth(30);
      static char attrib_offset[64] = "0";
      ImGui::InputText("Offset", attrib_offset, 64, ImGuiInputTextFlags_CharsDecimal);
      ImGui::SameLine();
      static char attrib_stride[64] = "3";
      ImGui::InputText("Stride", attrib_stride, 64, ImGuiInputTextFlags_CharsDecimal);
      ImGui::SameLine();
      ImGui::PopItemWidth();
      ImGui::SameLine();

      if (ImGui::Button("+"))
      {
        add_attrib(new_attrib_name, atoi(attrib_offset), atoi(attrib_stride));
      }
      ImGui::SameLine();
      if (ImGui::Button("-"))
      {
        delete_attrib(_ui_curr_attrib);
      }
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(ImGui::GetWindowSize().x);
      ImGui::PushItemWidth(ImGui::GetWindowSize().x);
      ImGui::ListBox("", &_ui_curr_attrib, _ui_attrib_names, _ui_attrib_names.size(), 5);
      ImGui::PopItemWidth();
      ImGui::EndChild();

      ImGui::Checkbox("Depth Read", &_depth_read);
      ImGui::SameLine();
      ImGui::Checkbox("Depth Write", &_depth_write);

      ImGui::PopItemWidth();
    }
    ImGui::EndChild();

    //render the node graph editor
    ImGui::SameLine();

    if (true)
    {
      if (!_tab_window.isInited())
      {
        static const char* tabNames[] = { "Vertex Shader", "Fragment Shader" };
        static const int numTabs = sizeof(tabNames) / sizeof(tabNames[0]);
        static const char* tabTooltips[numTabs] = { "Vertex Shader Node Graph", "Fragment / Pixel Shader Node Graph" };
        for (int i = 0; i < numTabs; i++)
        {
          _tab_window.addTabLabel(tabNames[i], tabTooltips[i], false, false, &_nge[i]); // see additional args to prevent a tab from closing and from dragging
        }
      }

      _tab_window.render();
    }
    else
    {
      _nge[0].render();
    }

    ImGui::End();
  }
}

void MaterialTool::open()
{
  _last_fname = open_file_dialog("bagasdf");

  Tool::MaterialBaker mb;
  Tool::TmpMaterial _ui_mat;

  FILE *fp = fopen(_last_fname.c_str(), "r");
  if (!fp) { return; }

  mxml_node_t *xml_tree = NULL;
  xml_tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
  assert(xml_tree);
  fclose(fp);

  mb.load_xml(xml_tree, _ui_mat, "", std::cout);

  //blast this data back into the ui data structure
  //Note: if I could figure out how to have ImGui read std::strings directly, this wouldn't
  //      be needed. unfortunately, it doesn't seem like I can do that without copying the
  //      string back every frame.
  strcpy(_material_name, _ui_mat._name.c_str());
  strcpy(_shader_name, _ui_mat._shader_name.c_str());
  sprintf(_vertex_size, "%i", _ui_mat._vertex_size);
  if (_ui_mat._target_buffer.compare("gbuffer") == 0)
  {
    _target_buffer = 0;
  }
  else if (_ui_mat._target_buffer.compare("distortion") == 0)
  {
    _target_buffer = 1;
  }
  else if (_ui_mat._target_buffer.compare("depth") == 0)
  {
    _target_buffer = 2; 
  }

  _blend_dst_mode = _blend_src_mode = 0; //no blending by default
  if (_ui_mat._blending)
  {
    switch (_ui_mat._src_blend_mode)
    {
    case GL_ZERO:
      _blend_src_mode = 1;
      break;
    case GL_ONE:
      _blend_src_mode = 2;
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      _blend_src_mode = 3;
      break;
    case GL_SRC_ALPHA_SATURATE:
      _blend_src_mode = 4;
      break;
    case GL_SRC_ALPHA:
      _blend_src_mode = 5;
      break;
    default:
      cout << "UNKNOWN GL BLEND MODE DETECTED!" << endl;
      break;
    }
    switch (_ui_mat._dst_blend_mode)
    {
    case GL_ZERO:
      _blend_dst_mode = 1;
      break;
    case GL_ONE:
      _blend_dst_mode = 2;
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      _blend_dst_mode = 3;
      break;
    case GL_SRC_ALPHA_SATURATE:
      _blend_dst_mode = 4;
      break;
    case GL_SRC_ALPHA:
      _blend_dst_mode = 5;
      break;
    default:
      cout << "UNKNOWN GL BLEND MODE DETECTED!" << endl;
      break;
    }
  }

  _culling_mode = 0; //no culling by default
  if (_ui_mat._backface_cull)
  {
    switch (_ui_mat._winding_order)
    {
    case GL_CCW:
      _culling_mode = 1;
      break;
    case GL_CW:
      _culling_mode = 2;
      break;
    default:
      cout << "UNKNOWN CULLING WINDING ORDER DETECTED!" << endl;
      break;
    }
  }

  _depth_read = _ui_mat._depth_read;
  _depth_write = _ui_mat._depth_write;

  _texture_names.clear();
  _ui_texture_names.clear();
  _ui_curr_texture = 0;
  for (uint32_t i = 0; i < _ui_mat._texture_names.size(); i++)
  {
    add_texture(_ui_mat._texture_names[i]);
  }

  _uniforms.clear();
  _ui_uniform_names.clear();
  _ui_uniform_type = 0;
  _ui_curr_uniform = 0;
  for (uint32_t i = 0; i < _ui_mat._uniforms.size(); i++)
  {
    add_uniform(_ui_mat._uniforms[i]._name, _ui_mat._uniforms[i]._type);
  }

  _attribs.clear();
  _ui_attrib_names.clear();
  _ui_curr_attrib = 0;
  _ui_attrib_type = 0;
  for (uint32_t i = 0; i < _ui_mat._attribs.size(); i++)
  {
    add_attrib(_ui_mat._attribs[i]._name, _ui_mat._attribs[i]._offset, _ui_mat._attribs[i]._stride);
  }
}

void MaterialTool::save_as(std::string fname)
{
  if (fname.size() == 0)
  {
    _last_fname = open_file_dialog("blah");
  }

  uint32_t version = 1;

  bool blending_enabled = (_blend_src_mode != 0 && _blend_dst_mode != 0);

  std::ofstream of;
  of.open(_last_fname.c_str());
  of << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
  of << "<material version=\"" << version << "\" name=\"" << _material_name << "\" vertex_size_bytes=\"" << _vertex_size << "\">" << endl;
  of << "\t<shader>" << _shader_name << "</shader>" << endl;
  of << "\t<target_buffer>" << RenderTargetNames[_target_buffer] << "</target_buffer>" << endl;
  of << "\t<alpha_blend src_mode=\"" << BlendModeNames[_blend_src_mode] << "\"";
  of << " dst_mode=\"" << BlendModeNames[_blend_dst_mode] << "\">";
  of <<  blending_enabled ? "true" : "false";
  of << "</alpha_blend>" << endl;
  of << "\t<depth_read>" << (_depth_read ? "true" : "false") << "</depth_read>" << endl;
  of << "\t<depth_write>" << (_depth_write ? "true" : "false") << "</depth_write>" << endl;
  of << "\t<backface_cull winding_order=\"" << CullingModeNames[_culling_mode] <<"\">";
  of << (_culling_mode != 0 ? "true" : "false") << "</backface_cull>" << endl;

  //textures
  for (uint32_t i = 0; i < _texture_names.size(); i++)
  {
    of << "\t<texture>" << _texture_names[i].c_str() << "</texture>" << endl;
  }

  //uniforms
  for (uint32_t i = 0; i < _uniforms.size(); i++)
  {
    of << "\t<uniform type=\"" << UniformTypeNames[_uniforms[i]._type];
    of << "\" name=\"" << _uniforms[i]._name << "\">" << 666.6 << "</uniform>" << endl;
  }

  //vertex attribs
  for (uint32_t i = 0; i < _attribs.size(); i++)
  {
    of << "\t<vertex_attrib offset=\"" << _attribs[i]._offset << "\" ";
    of << "stride=\"" << _attribs[i]._stride << "\">" << _attribs[i]._name << "</vertex_attrib>" << endl;
  }

  of << "</material>" << endl;
}

void MaterialTool::add_texture(std::string name)
{
  std::string ui_name = std::to_string(_texture_names.size()) + std::string(": ") + name;
  _ui_texture_names.push_back(ui_name);

  _texture_names.push_back(name);
}
void MaterialTool::delete_texture(uint32_t idx)
{
  if (idx < 0 || idx >= _ui_texture_names.size()) { return; }
  _texture_names.erase(_texture_names.begin() + idx);
  _ui_texture_names.erase(_ui_texture_names.begin() + idx);
}
void MaterialTool::add_uniform(std::string name, uint32_t type)
{
  std::string ui_name = std::to_string(_uniforms.size()) +
    std::string(": ") +
    name +
    std::string(" (") +
    UniformTypeNames[type] +
    ")";
  _ui_uniform_names.push_back(ui_name);

  UniformUI uni;
  uni._name = name;
  uni._type = type;
  _uniforms.push_back(uni);
}
void MaterialTool::delete_uniform(uint32_t idx)
{
  if (idx < 0 || idx >= _uniforms.size()) { return; }
  _uniforms.erase(_uniforms.begin() + idx);
  _ui_uniform_names.erase(_ui_uniform_names.begin() + idx);
}

void MaterialTool::add_attrib(std::string name, uint32_t offset, uint32_t stride)
{
  std::string ui_name = std::to_string(_attribs.size()) +
    std::string(": ") +
    name +
    std::string(" (offset: ") +
    std::to_string(offset) +
    std::string(", stride: ") +
    std::to_string(stride) +
    std::string(")");
  _ui_attrib_names.push_back(ui_name);

  AttributeUI attrib;
  attrib._name = name;
  attrib._offset = offset;
  attrib._stride = stride;
  _attribs.push_back(attrib);
}

void MaterialTool::delete_attrib(uint32_t idx)
{
  if (idx < 0 || idx >= _attribs.size()) { return; }
  _attribs.erase(_attribs.begin() + idx);
  _ui_attrib_names.erase(_ui_attrib_names.begin() + idx);
}

void MaterialTool::generate_glsl(int pipeline_stage, std::ostream &codex)
{
  //vertex shader
  ImVector<ImGui::Node *> vs_in_nodes,
                          fs_in_nodes,
                          out_nodes,
                          glsl_nodes,
                          uniform_nodes,
                          constant_nodes,
                          texture_nodes;
  
  _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_GLSL, &glsl_nodes);
  _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_UNIFORM_VARIABLE, &uniform_nodes);
  _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_CONSTANT_VARIABLE, &constant_nodes);
  _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_TEXTURE2D, &texture_nodes);
  _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_VS_INPUT, &vs_in_nodes);
  _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_FS_INPUT, &fs_in_nodes);

  MeshInputNode *in_node = NULL;
  if(pipeline_stage == 0)
  {
    _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_VS_OUTPUT, &out_nodes);

    assert(vs_in_nodes.size() == 1);
    in_node = (MeshInputNode *)vs_in_nodes[0];
  }
  else
  {
    _nge[pipeline_stage].getAllNodesOfType(SHADER_NODE_FS_OUTPUT, &out_nodes);

    assert(fs_in_nodes.size() == 1);
    in_node = (FSInputNode *)fs_in_nodes[0];
  }

  for (int i = 0; i < out_nodes.size(); i++) //really, there should only ever be one
  {
    VertexShaderOutputNode *vs_out_node = (VertexShaderOutputNode *)out_nodes[i];
    FragmentShaderOutputNode *fs_out_node = (FragmentShaderOutputNode *)out_nodes[i];
    codex << "// Generated VS Output: " << out_nodes[i]->getName() << endl;
    codex << "#version 450" << endl;
    codex << endl;

    //vertex input / "attribs"
    codex << "// vertex input / attribs" << endl;
    for (uint32_t j = 0; j < in_node->getNumOutputSlots(); j++)
    {
      in_node->variable_declaration(codex, j);
    }
    codex << endl;

    //input / uniform / engine variables
    codex << "// uniform / engine input variables" << endl;
    for (uint32_t j = 0; j < uniform_nodes.size(); j++)
    {
      UniformNode *uni_node = (UniformNode *)uniform_nodes[j];
      std::string uni_type = uni_node->get_var_type_name();
      codex << "in " << uni_type.c_str() << " " << uni_node->getName() << ";" << endl;
    }
    codex << endl;

    //textures
    codex << "// textures" << endl;
    for (uint32_t j = 0; j < texture_nodes.size(); j++)
    {
      TextureNode *tex_node = (TextureNode *)texture_nodes[j];
      std::string var_name = "tex_prefetch_" + std::to_string(j);
      tex_node->set_prefetch_name(var_name);
      tex_node->prep_for_code_generation();
      codex << "in sampler2D " << tex_node->getName() << ";" << endl;
    }
    codex << endl;

    //static consts
    codex << "// constant variables" << endl;
    for (uint32_t j = 0; j < constant_nodes.size(); j++)
    {
      ConstantNode *const_node = (ConstantNode *)constant_nodes[j];
      std::string const_type = const_node->get_var_type_name();
      codex << /*"const " <<*/ const_type.c_str() << " " << const_node->getName() <<" = ";
      std::string val_string = const_node->get_value_string();
      codex << val_string.c_str() << ";" << endl;
    }
    codex << endl;

    //output / varying variables
    //std::vector<std::string> out_var_names;
    //n->get_var_names(out_var_names);
    codex << "// output variables (to be passed to the fragment shader)" << endl;
    for (uint32_t j = 0; j < _varying_vars.size(); j++)
    {
      std::string type_name(GLSL_type_names[_varying_vars[j].first]);
      codex << "out " << type_name.c_str() << " " << _varying_vars[j].second.c_str() << ";" << endl;
    }

    //glsl function definitions
    for (uint32_t j = 0; j < glsl_nodes.size(); j++)
    {
      TextNode *text_node = (TextNode *)glsl_nodes[j];
      std::string var_name = "func_prefetch_" + std::to_string(j);
      text_node->set_prefetch_name(var_name);
      text_node->prep_for_code_generation();

      codex << endl;
      text_node->function_declaration(codex);
      codex << endl;
    }

    codex << endl;
    codex << "void main()" << endl;
    codex << "{" << endl;

    if (pipeline_stage == 0)
    {
      //prefetch textures
      vs_out_node->generate_prefetch_declarations("\t", codex, -1);

      //now trace back up the path
      vs_out_node->generate_glsl(codex, -1);
    }
    else
    {
      //prefetch textures
      fs_out_node->generate_prefetch_declarations("\t", codex, -1);

      //now trace back up the path
      fs_out_node->generate_glsl(codex, -1);
    }

    codex << "}" << endl;
  }
}