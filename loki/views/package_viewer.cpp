#include "package_viewer.h"
#include "ui_common.h"

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
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    Tool::PackageBaker pb;
    pb.parse_xml(tree, _pt, std::cout);
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