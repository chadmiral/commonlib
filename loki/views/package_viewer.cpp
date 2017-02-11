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
          load_package("../../../mundus/data/packages/pkg_test.pkg.xml");
        }
        if (ImGui::MenuItem("Save"))
        {
          if (_curr_pkg_fname.length() > 0)
          {
            save_package(_curr_pkg_fname);
          }
        }
        if (ImGui::MenuItem("Save As..."))
        {

        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    static uint32_t tree_group = 0;
    ImGui::BeginChild(tree_group, ImVec2(350, 0), true);
    {
      ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

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
    ImGui::BeginChild(edit_group, ImVec2(350, 0), true);
    {
      if (ImGui::Button("+"))
      {
        add_new_asset();
      }
      ImGui::SameLine();
      ImGui::Button("-"); ImGui::SameLine();
      ImGui::Combo("", &_curr_new_asset, Tool::Package_asset_names, Tool::NUM_PACKAGE_ASSET_TYPES);

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
  Tool::MeshTemplate *mt = &_pt._meshes[_ui_curr_selection[Tool::PACKAGE_ASSET_MESH]];
  strcpy(name_buffer, mt->_name.c_str());
  strcpy(fname_buffer, mt->_fname.c_str());
  
  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("File Name", fname_buffer, 256);
}

void PackageViewer::render_material_ui()
{
  char name_buffer[256], fname_buffer[256];
  Tool::MaterialTemplate *mt = &_pt._materials[_ui_curr_selection[Tool::PACKAGE_ASSET_MATERIAL]];
  strcpy(name_buffer, mt->_name.c_str());
  strcpy(fname_buffer, mt->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("File Name", fname_buffer, 256);
}

void PackageViewer::render_shader_ui()
{
  char name_buffer[256], fs_fname_buffer[256], vs_fname_buffer[256];
  Tool::ShaderTemplate *st = &_pt._shaders[_ui_curr_selection[Tool::PACKAGE_ASSET_SHADER]];
  strcpy(name_buffer, st->_name.c_str());
  strcpy(fs_fname_buffer, st->_fs_fname.c_str());
  strcpy(vs_fname_buffer, st->_vs_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("Vertex Shader", vs_fname_buffer, 256);
  ImGui::InputText("Fragment Shader", fs_fname_buffer, 256);
}

void PackageViewer::render_texture_ui()
{
  char name_buffer[256], fname_buffer[256];
  static int curr_format, wrap_u, wrap_v;

  Tool::TextureTemplate *tt = &_pt._textures[_ui_curr_selection[Tool::PACKAGE_ASSET_TEXTURE]];
  strcpy(name_buffer, tt->_name.c_str());
  strcpy(fname_buffer, tt->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("File Name", fname_buffer, 256);
  ImGui::Combo("Compression", &curr_format, "None\0DXT3\0DXT5\0BC1\0BC3\0BC7");

  ImGui::Text("Wrap Mode");
  ImGui::PushItemWidth(120);
  ImGui::Combo("U", &wrap_u, "REPEAT\0CLAMP\0CLAMP_BORDER"); ImGui::SameLine();
  ImGui::Combo("V", &wrap_v, "REPEAT\0CLAMP\0CLAMP_BORDER");
  ImGui::PopItemWidth();
}

void PackageViewer::render_animation_ui()
{
  char name_buffer[256], fname_buffer[256];
  Tool::AnimationTemplate *at = &_pt._animations[_ui_curr_selection[Tool::PACKAGE_ASSET_ANIMATION]];
  strcpy(name_buffer, at->_name.c_str());
  strcpy(fname_buffer, at->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("File Name", fname_buffer, 256);
}

void PackageViewer::render_skeleton_ui()
{
  char name_buffer[256], fname_buffer[256];
  Tool::SkeletonTemplate *st = &_pt._skeletons[_ui_curr_selection[Tool::PACKAGE_ASSET_SKELETON]];
  strcpy(name_buffer, st->_name.c_str());
  strcpy(fname_buffer, st->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("File Name", fname_buffer, 256);
}

void PackageViewer::render_ui_layout_ui()
{
  char name_buffer[256], fname_buffer[256];
  Tool::UILayoutTemplate *ut = &_pt._ui_layouts[_ui_curr_selection[Tool::PACKAGE_ASSET_UI_LAYOUT]];
  strcpy(name_buffer, ut->_name.c_str());
  strcpy(fname_buffer, ut->_fname.c_str());

  ImGui::InputText("Name", name_buffer, 256);
  ImGui::InputText("File Name", fname_buffer, 256);
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
  switch (_curr_new_asset)
  {
  case Tool::PACKAGE_ASSET_MESH:
  {
    Tool::MeshTemplate mt;
    mt._name = "NewMesh";
    mt._fname = "";
    _pt._meshes.push_back(mt);
    _ui_mesh_names.push_back(mt._name);
    break;
  }
  case Tool::PACKAGE_ASSET_MATERIAL:
  {
    Tool::MaterialTemplate mt;
    mt._name = "NewMaterial";
    mt._fname = "";
    _pt._materials.push_back(mt);
    _ui_material_names.push_back(mt._name);
    break;
  }
  case Tool::PACKAGE_ASSET_SHADER:
  {
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
    Tool::AnimationTemplate at;
    at._name = "NewAnimation";
    at._fname = "";
    _pt._animations.push_back(at);
    _ui_animation_names.push_back(at._name);
    break;
  }
  case Tool::PACKAGE_ASSET_SKELETON:
  {
    Tool::SkeletonTemplate st;
    st._name = "NewSkeleton";
    st._fname = "";
    _pt._skeletons.push_back(st);
    _ui_skeleton_names.push_back(st._name);
    break;
  }
  case Tool::PACKAGE_ASSET_UI_LAYOUT:
  {
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