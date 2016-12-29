#include "animation_tool.h"
#include "math_utility.h"

using namespace Math;
using namespace Animation;

AnimationTool::AnimationTool()
{
  _anim = NULL;
  _visible = true;
  _data_root = "c:\\cygwin64\\home\\chandra\\code\\mundus\\data";
  _selected_bone = 0;
}

void AnimationTool::build_skeleton_tree()
{
  //std::vector<UIBone *> all_bones;
  assert(_anim);
  _all_bones.clear();
  for (uint32_t i = 0; i < _anim->_tracks.size(); i++)
  {
    UIBone *b = new UIBone;
    b->_bone_id = _anim->_tracks[i]._bone_id;
    b->_parent = NULL; // TODO (although need skeleton loaded)
    _all_bones.push_back(b);
  }
}

void AnimationTool::load_animation()
{
  char szFile[100];
  OPENFILENAME ofn;

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);

  if (_anim) { delete _anim; }
  _anim = new SkeletonAnimation;

  FILE *f;
  FOPEN(f, ofn.lpstrFile, "rb");
  if (f)
  {
    _anim->read_from_file(f);
    _anim_fname = ofn.lpstrFile + _data_root.length();
    build_skeleton_tree();
  }
  //_anim->init();
}

void AnimationTool::render_menu()
{
  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Open Animation...", NULL, false))
      {
        load_animation();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

void AnimationTool::render_animation_curves()
{
  //x, y, z, w
  char *axis_names[] = { "w", "x", "y", "z" };
  char *transform_names[] = { "pos", "rot", "scale" };

  for (uint32_t transform_idx = 0; transform_idx < 3; transform_idx++)
  {
    //uint32_t xyz_idx = 1;
    for (uint32_t xyz_idx = (transform_idx == 1 ? 0 : 1); xyz_idx < 4; xyz_idx++)
    {
      Float2 curve_range(1e10f, -1e10f);
      uint32_t curr_track = _selected_bone;
      uint32_t num_frames = 100;
      float *curve_values = NULL;
      if (_anim)
      {
        num_frames = _anim->_tracks[curr_track]._pos_frames.size();
        curve_values = new float[num_frames];
        for (uint32_t i = 0; i < num_frames; i++)
        {
          switch (transform_idx)
          {
          case 0:
            curve_values[i] = _anim->_tracks[curr_track]._pos_frames[i]._pos[xyz_idx];
            break;
          case 1:
            curve_values[i] = _anim->_tracks[curr_track]._rot_frames[i]._rot[xyz_idx];
            break;
          case 2:
            curve_values[i] = _anim->_tracks[curr_track]._scale_frames[i]._scale[xyz_idx];
            break;
          default:
            curve_values[i] = 0.0f;
          }

          if (curve_values[i] > curve_range[1]) { curve_range[1] = curve_values[i]; }
          if (curve_values[i] < curve_range[0]) { curve_range[0] = curve_values[i]; }
        }
      }
      else
      {
        curve_range[0] = 0.0f;
        curve_range[1] = 1.0f;
        curve_values = new float[num_frames];
        for (uint32_t i = 0; i < num_frames; i++)
        {
          curve_values[i] = (float)sin(4.0f * M_PI * (float)i / (float)num_frames);
        }
      }

      ImVec2 view_size = ImGui::GetWindowSize();
      view_size.x -= 20;
      view_size.y = 60;

      char plot_label[256];
      sprintf(plot_label, "%s: %s (%.6f, %.6f)", transform_names[transform_idx], axis_names[xyz_idx], curve_range[0], curve_range[1]);

      char frame_label[128];
      sprintf(frame_label, "%d frames", num_frames);

      ImGui::Text(plot_label);
      ImGui::PlotLines("", curve_values, num_frames, 0, frame_label, curve_range[0], curve_range[1], view_size);
      delete curve_values;
    }
  }
}

void AnimationTool::render()
{
  if (_visible)
  {
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Animation Tools", &_visible, ImGuiWindowFlags_MenuBar);
    ImGui::Text("");

    render_menu();

    if (ImGui::TreeNode("Bones"))
    {
      ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                      ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                      ImGuiTreeNodeFlags_Selected;
                                      

      for (uint32_t i = 0; i < _all_bones.size(); i++)
      {
        ImGui::TreeNodeEx((void*)(intptr_t)_selected_bone, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, "Bone %d", i);
        if (ImGui::IsItemClicked())
        {
          _selected_bone = i;
        }
      }

      ImGui::TreePop();
    }

    render_animation_curves();

    //uint32_t transform_idx = 1;

    //animation scrubber
    static int i1 = 0;
    //ImVec2 scrub_size(10, 10);
    //ImGui::SetNextWindowSize(scrub_size);
    //ImGui::SliderInt("", &i1, 0, num_frames);
    //ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

    ImGui::End();
  }
}