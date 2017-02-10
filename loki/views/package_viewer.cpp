#include <fstream>
#include "package_viewer.h"
#include "ui_common.h"

using namespace std;

PackageViewer::PackageViewer() : LokiView()
{
  _ui_curr_mesh = 0;
  _ui_curr_material = 0;
  _ui_curr_shader = 0;
  _ui_curr_texture = 0;
  _ui_curr_animation = 0;
  _ui_curr_skeleton = 0;
  _ui_curr_ui_layout = 0;
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

    if (ImGui::TreeNode("Assets"))
    {
      ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
      
      if (ImGui::TreeNode("Meshes"))
      {
        ImGui::ListBox("", &_ui_curr_mesh, _ui_mesh_names, _ui_mesh_names.size(), 5);
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Materials"))
      {
        ImGui::ListBox("", &_ui_curr_material, _ui_material_names, _ui_material_names.size(), 5);
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Shaders"))
      {
        ImGui::ListBox("", &_ui_curr_shader, _ui_shader_names, _ui_shader_names.size(), 5);
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Textures"))
      {
        ImGui::ListBox("", &_ui_curr_texture, _ui_texture_names, _ui_texture_names.size(), 5);
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Animations"))
      {
        ImGui::ListBox("", &_ui_curr_animation, _ui_animation_names, _ui_animation_names.size(), 5);
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Skeletons"))
      {
        ImGui::ListBox("", &_ui_curr_skeleton, _ui_skeleton_names, _ui_skeleton_names.size(), 5);
        ImGui::TreePop();
      }
      ImGui::TreePop();

      if (ImGui::TreeNode("UI Layouts"))
      {
        ImGui::ListBox("", &_ui_curr_ui_layout, _ui_ui_layout_names, _ui_ui_layout_names.size(), 5);
        ImGui::TreePop();
      }
    }

    ImGui::End();
  }
}