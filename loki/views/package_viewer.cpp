#include "package_viewer.h"

void PackageViewer::load_package(std::string pkg_fname)
{
  //clear out the current package template
  memset(&_pt, 0, sizeof(Tool::PackageTemplate));


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

        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    if (ImGui::TreeNode("Assets"))
    {
      if (ImGui::TreeNode("Shaders"))
      {
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Textures"))
      {
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }

    ImGui::End();
  }
}