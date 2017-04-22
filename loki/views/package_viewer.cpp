#include <fstream>
#include "package_viewer.h"
#include "ui_common.h"

using namespace std;

PackageViewer::PackageViewer() : LokiView()
{
  reset_selection(-1);

  _curr_new_asset = 0;
}

void PackageViewer::load_package(std::string pkg_fname)
{
  //clear out the current package template
  memset(&_pt, 0, sizeof(Tool::PackageTemplate));

  FILE *fp = fopen(pkg_fname.c_str(), "r");
  if (fp)
  {
    _curr_pkg_fname = pkg_fname;

    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    mxml_node_t *node = mxmlFindElement(tree, tree, "package", "version", NULL, MXML_DESCEND);
    if (node)
    {
      Tool::PackageBaker pb;
      _pt._version = atoi(mxmlElementGetAttr(node, "version"));
      pb.parse_xml(tree, _pt, std::cout);
    }
  }

  //update ui data structures
  _ui_mesh_names.clear();
  _ui_material_names.clear();
  _ui_shader_names.clear();
  _ui_texture_names.clear();
  _ui_animation_names.clear();
  _ui_skeleton_names.clear();
  _ui_ui_layout_names.clear();

  for (uint32_t i = 0; i < _pt._meshes.size(); i++)
  {
    _ui_mesh_names.push_back(_pt._meshes[i]._name);
  }
  
  for (uint32_t i = 0; i < _pt._materials.size(); i++)
  {
    _ui_material_names.push_back(_pt._materials[i]._name);
  }

  for (uint32_t i = 0; i < _pt._shaders.size(); i++)
  {
    _ui_shader_names.push_back(_pt._shaders[i]._name);
  }
  
  for (uint32_t i = 0; i < _pt._textures.size(); i++)
  {
    _ui_texture_names.push_back(_pt._textures[i]._name);
  }

  for (uint32_t i = 0; i < _pt._animations.size(); i++)
  {
    _ui_animation_names.push_back(_pt._animations[i]._name);
  }

  for (uint32_t i = 0; i < _pt._skeletons.size(); i++)
  {
    _ui_skeleton_names.push_back(_pt._skeletons[i]._name);
  }

  for (uint32_t i = 0; i < _pt._ui_layouts.size(); i++)
  {
    _ui_ui_layout_names.push_back(_pt._ui_layouts[i]._name);
  }
}

void PackageViewer::save_package(std::string pkg_fname)
{
  std::ofstream of;
  
  //xml header + file version
  of.open(pkg_fname.c_str());
  of << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>" << endl;
  of << "<package version=\"" << _pt._version << "\">" << endl;
  of << endl;

  //root dir + path + binary output file
  of << "<root_dir>" << _pt._root_dir << "</root_dir>" << endl;
  of << "<path>" << _pt._path << "</path>" << endl;
  of << "<output_file>" << _pt._output_file << "</output_file>" << endl;
  of << endl;

  //meshes
  for (uint32_t i = 0; i < _pt._meshes.size(); i++)
  {
    of << "<mesh name=\"" << _pt._meshes[i]._name << "\">" << _pt._meshes[i]._fname << "</mesh>" << endl;
  }
  of << endl;

  //materials
  for (uint32_t i = 0; i < _pt._materials.size(); i++)
  {
    of << "<material name=\"" << _pt._materials[i]._name << "\">" << _pt._materials[i]._fname << "</material>" << endl;
  }
  of << endl;

  //shaders
  for (uint32_t i = 0; i < _pt._shaders.size(); i++)
  {
    of << "<shader name=\"" << _pt._shaders[i]._name << "\">" << endl;
    of << "\t<vertex_shader>" << _pt._shaders[i]._vs_fname << "</vertex_shader>" << endl;
    of << "\t<fragment_shader>" << _pt._shaders[i]._fs_fname << "</fragment_shader>" << endl;
    of << "</shader>" << endl;
  }
  of << endl;

  //textures
  for (uint32_t i = 0; i < _pt._textures.size(); i++)
  {
    of << "<texture name=\"" << _pt._textures[i]._name << "\"";
    if (_pt._textures[i]._format.length() > 0)
    {
      of << " format=\"" << _pt._textures[i]._format << "\"";
    }
    if (_pt._textures[i]._wrap_u.length() > 0)
    {
      of << " wrap_u=\"" << _pt._textures[i]._wrap_u << "\"";
    }
    if (_pt._textures[i]._wrap_v.length() > 0)
    {
      of << " wrap_v=\"" << _pt._textures[i]._wrap_v << "\"";
    }
    of << ">" << _pt._textures[i]._fname << "</texture>" << endl;
  }
  of << endl;

  //animations
  for (uint32_t i = 0; i < _pt._animations.size(); i++)
  {
    of << "<animation name=\"" << _pt._animations[i]._name << "\">" << _pt._animations[i]._fname << "</animation>" << endl;
  }
  of << endl;

  //skeletons
  for (uint32_t i = 0; i < _pt._skeletons.size(); i++)
  {
    of << "<skeleton name=\"" << _pt._skeletons[i]._name << "\">" << _pt._skeletons[i]._fname << "</skeleton>" << endl;
  }
  of << endl;

  //ui layouts
  for (uint32_t i = 0; i < _pt._ui_layouts.size(); i++)
  {
    of << "<ui_layout name=\"" << _pt._ui_layouts[i]._name << "\">" << _pt._ui_layouts[i]._fname << "</ui_layout>" << endl;
  }

  of << "</package>" << endl;
  of.close();
}

void PackageViewer::save_as()
{
  std::string fname = open_file_dialog("/packages/", "Game Package\0*.pkg.xml\0");
  save_package(fname);
}

void PackageViewer::bake_package()
{
  Tool::PackageBaker pb;

  FILE *fp = fopen(_curr_pkg_fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    fclose(fp);

    std::string output_fname = _curr_pkg_fname + ".bin";
    
    mxml_node_t *node = mxmlFindElement(tree, tree, "package", "version", NULL, MXML_DESCEND);
    if (node)
    {
      Tool::PackageTemplate pt;
      pt._version = atoi(mxmlElementGetAttr(node, "version"));
      pb.bake(tree, output_fname, pt, std::cout, "");
    }
  }
}

void PackageViewer::move_selected_asset(Tool::PackageAssetType pat, int32_t amt)
{
  int sel = _ui_curr_selection[pat] + amt;
  if (sel < 0) { return; }
  switch (pat)
  {
  case Tool::PACKAGE_ASSET_ANIMATION:
    if (sel >= _pt._animations.size()) { return; }
    break;
  case Tool::PACKAGE_ASSET_MATERIAL:
    if (sel >= _pt._materials.size()) { return; }
    break;
  case Tool::PACKAGE_ASSET_MESH:
    if (sel >= _pt._meshes.size()) { return; }
    break;
  case Tool::PACKAGE_ASSET_SHADER:
    if (sel >= _pt._shaders.size()) { return; }
    break;
  case Tool::PACKAGE_ASSET_SKELETON:
    if (sel >= _pt._skeletons.size()) { return; }
    break;
  case Tool::PACKAGE_ASSET_TEXTURE:
  {
    if (sel >= _pt._textures.size()) { return; }
    Tool::TextureTemplate tt = _pt._textures[_ui_curr_selection[pat]];
    _pt._textures.erase(_pt._textures.begin() + _ui_curr_selection[pat]);
    _ui_texture_names.erase(_ui_texture_names.begin() + _ui_curr_selection[pat]);
    _pt._textures.insert(_pt._textures.begin() + amt, tt);
    _ui_texture_names.insert(_ui_texture_names.begin() + amt, tt._name);
    break;
  }
  case Tool::PACKAGE_ASSET_UI_LAYOUT:
    if (sel >= _pt._ui_layouts.size()) { return; }
    break;
  default:
    return;
  }
}

void PackageViewer::render()
{
  if (visible)
  {
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Package Builder", &visible, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Open Package File...", NULL, false))
        {
          //TODO: allow open file dialog
          std::string fname = open_file_dialog("/packages/", "Game Package\0*.pkg.xml\0");
          load_package(fname);
        }
        if (ImGui::MenuItem("Save"))
        {
          if (_curr_pkg_fname.length() == 0)
          {
            save_as();
          }
          else
          {
            save_package(_curr_pkg_fname);
          }
        }
        if (ImGui::MenuItem("Save As..."))
        {
          save_as();
        }
        if (ImGui::MenuItem("Bake"))
        {
          if (_curr_pkg_fname.length() == 0)
          {
            save_as();
          }
          bake_package();
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    static uint32_t tree_group = 0;
    ImGui::BeginChild(tree_group, ImVec2(350, 0), true);
    {
      if (ImGui::Button("+"))
      {
        add_new_asset();
      }
      ImGui::SameLine();
      ImGui::Button("-"); ImGui::SameLine();
      ImGui::Combo("", &_curr_new_asset, Tool::Package_asset_names, Tool::NUM_PACKAGE_ASSET_TYPES);

      ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

      //ImGui::SetNextTreeNodeOpen(_ui_curr_selection[Tool::PACKAGE_ASSET_MESH] != -1);
      if (ImGui::TreeNode("Meshes"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_MESH], _ui_mesh_names, _ui_mesh_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_MESH);
        }
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Materials"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_MATERIAL], _ui_material_names, _ui_material_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_MATERIAL);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Shaders"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_SHADER], _ui_shader_names, _ui_shader_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_SHADER);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Textures"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_TEXTURE], _ui_texture_names, _ui_texture_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_TEXTURE);
        }
        ImGui::SameLine();
        if (ImGui::Button("^"))
        {
          move_selected_asset(Tool::PACKAGE_ASSET_TEXTURE, -1);
        }
        ImGui::SameLine();
        if (ImGui::Button("v"))
        {
          move_selected_asset(Tool::PACKAGE_ASSET_TEXTURE, 1);
        }
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Animations"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_ANIMATION], _ui_animation_names, _ui_animation_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_ANIMATION);
        }
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Skeletons"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_SKELETON], _ui_skeleton_names, _ui_skeleton_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_SKELETON);
        }
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("UI Layouts"))
      {
        if (ImGui::ListBox("", &_ui_curr_selection[Tool::PACKAGE_ASSET_UI_LAYOUT], _ui_ui_layout_names, _ui_ui_layout_names.size(), 5))
        {
          reset_selection(Tool::PACKAGE_ASSET_UI_LAYOUT);
        }
        ImGui::TreePop();
      }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    static uint32_t edit_group = 1;
    ImGui::BeginChild(edit_group, ImVec2(450, 0), true);
    {
      Tool::PackageAssetType pat = get_curr_asset_type();
      
      switch (pat)
      {
      case Tool::PACKAGE_ASSET_MESH:
        render_mesh_ui();
        break;
      case Tool::PACKAGE_ASSET_MATERIAL:
        render_material_ui();
        break;
      case Tool::PACKAGE_ASSET_SHADER:
        render_shader_ui();
        break;
      case Tool::PACKAGE_ASSET_TEXTURE:
        render_texture_ui();
        break;
      case Tool::PACKAGE_ASSET_ANIMATION:
        render_animation_ui();
        break;
      case Tool::PACKAGE_ASSET_SKELETON:
        render_skeleton_ui();
        break;
      case Tool::PACKAGE_ASSET_UI_LAYOUT:
        render_ui_layout_ui();
        break;
      }
    }
    ImGui::EndChild();

    ImGui::End();
  }
}

void PackageViewer::render_mesh_ui()
{
  char name_buffer[256], fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_MESH];
  Tool::MeshTemplate *mt = &_pt._meshes[idx];
  strcpy(name_buffer, mt->_name.c_str());
  strcpy(fname_buffer, mt->_fname.c_str());
  
  ImGui::InputText("Name", name_buffer, 256);

  _ui_mesh_names[idx] = name_buffer; //terrible, fix this
  mt->_name = name_buffer; 
  
  if (ImGui::Button("..."))
  {
    mt->_fname = make_filename_relative(open_file_dialog("/meshes", "Mesh\0*.brick\0All\*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(mt->_fname.c_str());
}

void PackageViewer::render_material_ui()
{
  char name_buffer[256], fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_MATERIAL];
  Tool::MaterialTemplate *mt = &_pt._materials[idx];
  strcpy(name_buffer, mt->_name.c_str());
  strcpy(fname_buffer, mt->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);

  _ui_material_names[idx] = name_buffer; //terrible, fix this
  mt->_name = name_buffer;

  if (ImGui::Button("..."))
  {
    mt->_fname = make_filename_relative(open_file_dialog("/materials", "Material\0*.mat\0All\0*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(mt->_fname.c_str());
}

void PackageViewer::render_shader_ui()
{
  char name_buffer[256], vs_fname_buffer[256], fs_fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_SHADER];
  Tool::ShaderTemplate *st = &_pt._shaders[idx];
  strcpy(name_buffer, st->_name.c_str());
  strcpy(vs_fname_buffer, st->_vs_fname.c_str());
  strcpy(fs_fname_buffer, st->_fs_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);

  _ui_shader_names[idx] = name_buffer; //terrible, fix this
  st->_name = name_buffer;

  ImGui::Text("Vertex Shader");
  if (ImGui::Button("..."))
  {
    st->_vs_fname = make_filename_relative(open_file_dialog("/shaders", "Vertex Shader\0*.vs\0All\0*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(st->_vs_fname.c_str());

  ImGui::Text("Fragment Shader");
  ImGui::PushID(64341); //won't detect button click otherwise
  if (ImGui::Button("..."))
  {
    st->_fs_fname = make_filename_relative(open_file_dialog("/shaders", "Fragment Shader\0*.fs\0All\0*.*\0"));
  }
  ImGui::PopID();
  ImGui::SameLine();
  ImGui::Text(st->_fs_fname.c_str());
}

void PackageViewer::render_texture_ui()
{
  char name_buffer[256], fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_TEXTURE];
  Tool::TextureTemplate *tt = &_pt._textures[idx];
  strcpy(name_buffer, tt->_name.c_str());
  strcpy(fname_buffer, tt->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);

  _ui_texture_names[idx] = name_buffer; //terrible, fix this
  tt->_name = name_buffer;

  if (ImGui::Button("..."))
  {
    tt->_fname = make_filename_relative(open_file_dialog("/meshes", "Image Files\0*.tga;*.png;*.jpg;*tif;*.tiff\0All\*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(tt->_fname.c_str());

  static int curr_format = 0;
  static int curr_wrap_u = 0;
  static int curr_wrap_v = 0;

  if (tt->_format.length() == 0) { curr_format = 0; }
  else if (!tt->_format.compare("DXT3")) { curr_format = 1; }
  else if (!tt->_format.compare("DXT5")) { curr_format = 2; }
  ImGui::Combo("Compression", &curr_format, "None\0DXT3\0DXT5\0BC1\0BC3\0BC7");
  switch (curr_format)
  {
  case 0:
    tt->_format = "";
    break;
  case 1:
    tt->_format = "DXT3";
    break;
  case 2:
    tt->_format = "DXT5";
    break;
  default:
    tt->_format = "";
    break;
  }

  if (tt->_wrap_u.length() == 0) { curr_wrap_u = 0; }
  else if (!tt->_wrap_u.compare("CLAMP")) { curr_wrap_u = 1; }
  else if (!tt->_wrap_u.compare("CLAMP_BORDER")) { curr_wrap_u = 2; }

  if (tt->_wrap_v.length() == 0) { curr_wrap_v = 0; }
  else if (!tt->_wrap_v.compare("CLAMP")) { curr_wrap_v = 1; }
  else if (!tt->_wrap_v.compare("CLAMP_BORDER")) { curr_wrap_v = 2; }
  ImGui::Text("Wrap Mode");
  ImGui::PushItemWidth(140);
  ImGui::Combo("U", &curr_wrap_u, "REPEAT\0CLAMP\0CLAMP_BORDER"); ImGui::SameLine();
  ImGui::Combo("V", &curr_wrap_v, "REPEAT\0CLAMP\0CLAMP_BORDER");
  ImGui::PopItemWidth();

  switch (curr_wrap_u)
  {
  case 0:
    tt->_wrap_u = "";
    break;
  case 1:
    tt->_wrap_u = "CLAMP";
    break;
  case 2:
    tt->_wrap_u = "CLAMP_BORDER";
    break;
  default:
    tt->_wrap_u = "";
    break;
  }

  switch (curr_wrap_v)
  {
  case 0:
    tt->_wrap_v = "";
    break;
  case 1:
    tt->_wrap_v = "CLAMP";
    break;
  case 2:
    tt->_wrap_v = "CLAMP_BORDER";
    break;
  default:
    tt->_wrap_v = "";
    break;
  }
}

void PackageViewer::render_animation_ui()
{
  char name_buffer[256], fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_ANIMATION];
  Tool::AnimationTemplate *at = &_pt._animations[idx];
  strcpy(name_buffer, at->_name.c_str());
  strcpy(fname_buffer, at->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);

  _ui_animation_names[idx] = name_buffer; //terrible, fix this
  at->_name = name_buffer;

  if (ImGui::Button("..."))
  {
    at->_fname = make_filename_relative(open_file_dialog("/animations", "Animation\0*.anim\0All\0*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(at->_fname.c_str());
}

void PackageViewer::render_skeleton_ui()
{
  char name_buffer[256], fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_SKELETON];
  Tool::SkeletonTemplate *st = &_pt._skeletons[idx];
  strcpy(name_buffer, st->_name.c_str());
  strcpy(fname_buffer, st->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);

  _ui_skeleton_names[idx] = name_buffer; //terrible, fix this
  st->_name = name_buffer;

  if (ImGui::Button("..."))
  {
    st->_fname = make_filename_relative(open_file_dialog("/skeletons", "Skeleton\0*.bones\0All\0*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(st->_fname.c_str());
}

void PackageViewer::render_ui_layout_ui()
{
  char name_buffer[256], fname_buffer[256];
  uint32_t idx = _ui_curr_selection[Tool::PACKAGE_ASSET_UI_LAYOUT];
  Tool::UILayoutTemplate *ut = &_pt._ui_layouts[idx];
  strcpy(name_buffer, ut->_name.c_str());
  strcpy(fname_buffer, ut->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);

  _ui_ui_layout_names[idx] = name_buffer; //terrible, fix this
  ut->_name = name_buffer;

  if (ImGui::Button("..."))
  {
    ut->_fname = make_filename_relative(open_file_dialog("/ui", "UI Layout\0*.layout.xml\0All\0*.*\0"));
  }
  ImGui::SameLine();
  ImGui::Text(ut->_fname.c_str());
}

Tool::PackageAssetType PackageViewer::get_curr_asset_type()
{
  for (uint32_t i = 0; i < Tool::NUM_PACKAGE_ASSET_TYPES; i++)
  {
    if (_ui_curr_selection[i] >= 0)
    {
      return (Tool::PackageAssetType)i;
    }
  }
  return Tool::INVALID_PACKAGE_ASSET;
}

void PackageViewer::reset_selection(int32_t mask_id)
{
  for (int32_t i = 0; i < (int32_t)Tool::NUM_PACKAGE_ASSET_TYPES; i++)
  {
    if (i != mask_id)
    {
      _ui_curr_selection[i] = -1;
    }
  }
}

void PackageViewer::add_new_asset()
{
  reset_selection(_curr_new_asset);
  switch (_curr_new_asset)
  {
  case Tool::PACKAGE_ASSET_MESH:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_mesh_names.size();
    Tool::MeshTemplate mt;
    mt._name = "NewMesh";
    mt._fname = "";
    _pt._meshes.push_back(mt);
    _ui_mesh_names.push_back(mt._name);
    break;
  }
  case Tool::PACKAGE_ASSET_MATERIAL:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_material_names.size();
    Tool::MaterialTemplate mt;
    mt._name = "NewMaterial";
    mt._fname = "";
    _pt._materials.push_back(mt);
    _ui_material_names.push_back(mt._name);
    break;
  }
  case Tool::PACKAGE_ASSET_SHADER:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_shader_names.size();
    Tool::ShaderTemplate st;
    st._name = "NewShader";
    st._vs_fname = "";
    st._fs_fname = "";
    _pt._shaders.push_back(st);
    _ui_shader_names.push_back(st._name);
    break;
  }
  case Tool::PACKAGE_ASSET_TEXTURE:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_texture_names.size();
    Tool::TextureTemplate tt;
    tt._name = "NewTexture";
    tt._fname = "";
    tt._format = "DXT5";
    tt._wrap_u = "";
    tt._wrap_v = "";
    _pt._textures.push_back(tt);
    _ui_texture_names.push_back(tt._name);
    break;
  }
  case Tool::PACKAGE_ASSET_ANIMATION:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_animation_names.size();
    Tool::AnimationTemplate at;
    at._name = "NewAnimation";
    at._fname = "";
    _pt._animations.push_back(at);
    _ui_animation_names.push_back(at._name);
    break;
  }
  case Tool::PACKAGE_ASSET_SKELETON:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_skeleton_names.size();
    Tool::SkeletonTemplate st;
    st._name = "NewSkeleton";
    st._fname = "";
    _pt._skeletons.push_back(st);
    _ui_skeleton_names.push_back(st._name);
    break;
  }
  case Tool::PACKAGE_ASSET_UI_LAYOUT:
  {
    _ui_curr_selection[_curr_new_asset] = _ui_ui_layout_names.size();
    Tool::UILayoutTemplate ut;
    ut._name = "NewUILayout";
    ut._fname = "";
    _pt._ui_layouts.push_back(ut);
    _ui_ui_layout_names.push_back(ut._name);
    break;
  }
  default:
    assert(false);
  }
}