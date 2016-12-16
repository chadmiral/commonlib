#include <iostream>

#include "animation_baker.h"
//#include "animation.h"
#include "skeleton.h"

using namespace Tool;
using namespace std;
using namespace Math;
using namespace Animation;

struct TmpAnimCurve
{
  std::string         _bone_name;
  BoneTransformType   _transform_type;
  BoneTransformAxis   _transform_axis;
  std::vector<Float2> _frames;
};

void AnimationBaker::bake(mxml_node_t *tree, std::string output_fname, std::string tabs)
{
  tabs += "\t";
  const char *buffer = NULL;

  cout << tabs.c_str() << "Parsing animation xml...";
  tabs += "\t";

  mxml_node_t *animation_node = mxmlFindElement(tree, tree, "animation", NULL, NULL, MXML_DESCEND);
  assert(animation_node);

  buffer = mxmlElementGetAttr(animation_node, "version");
  uint32_t version = atoi(buffer);

  cout << "version " << version << endl;

  buffer = mxmlElementGetAttr(animation_node, "length_frames");
  uint32_t length_frames = atoi(buffer);
  cout << tabs.c_str() << "# frames: " << length_frames << endl;

  buffer = mxmlElementGetAttr(animation_node, "fps");
  uint32_t fps = atoi(buffer);
  cout << tabs.c_str() << "fps: " << fps << endl;

  //now loop through all the anim_curves
  mxml_node_t *curve_node = NULL;
  mxml_node_t *start_node = tree;

  std::vector<TmpAnimCurve> tracks;
  std::vector<std::string> bone_names;

  do
  {
    curve_node = mxmlFindElement(start_node, tree, "anim_curve", NULL, NULL, MXML_DESCEND);

    if (curve_node)
    {
      TmpAnimCurve tac;

      buffer = mxmlElementGetAttr(curve_node, "bone");
      std::string bone_name = buffer;
      //cout << tabs.c_str() << "bone: " << bone_name.c_str() << endl;
      tac._bone_name = bone_name;

      //add the bone, if unique
      bool found = false;
      for (uint32_t i = 0; i < bone_names.size(); i++)
      {
        if (bone_names[i].compare(bone_name) == 0) { found = true; break; }
      }
      if (!found) { bone_names.push_back(bone_name); }

      buffer = mxmlElementGetAttr(curve_node, "data_type");
      std::string data_type = buffer;
      //cout << tabs.c_str() << "data type: " << data_type.c_str() << endl;

      tac._transform_type = BONE_TRANSFORM_INVALID;
      if (data_type.compare("rotation_quaternion") == 0)
      {
        tac._transform_type = BONE_TRANSFORM_ROT_QUAT;
      }
      else if (data_type.compare("location") == 0)
      {
        tac._transform_type = BONE_TRANSFORM_LOC;
      }
      else if (data_type.compare("scale") == 0)
      {
        tac._transform_type = BONE_TRANSFORM_SCALE;
      }
      assert(tac._transform_type != BONE_TRANSFORM_INVALID);

      buffer = mxmlElementGetAttr(curve_node, "axis");
      std::string axis = buffer;
      //cout << tabs.c_str() << "axis: " << axis.c_str() << endl;

      tac._transform_axis = BONE_TRANSFORM_AXIS_INVALID;
      if (axis.compare("x") == 0)
      {
        tac._transform_axis = BONE_TRANSFORM_AXIS_X;
      }
      else if (axis.compare("y") == 0)
      {
        tac._transform_axis = BONE_TRANSFORM_AXIS_Y;
      }
      else if (axis.compare("z") == 0)
      {
        tac._transform_axis = BONE_TRANSFORM_AXIS_Z;
      }
      else if (axis.compare("w") == 0)
      {
        tac._transform_axis = BONE_TRANSFORM_AXIS_W;
      }
      assert(tac._transform_axis != BONE_TRANSFORM_AXIS_INVALID);

      mxml_node_t *frame_node = NULL;
      start_node = curve_node;
      do
      {
        frame_node = mxmlFindElement(start_node, curve_node, "frame", NULL, NULL, MXML_DESCEND);
        if (frame_node)
        {
          Float2 f = mxml_read_float2(frame_node->child);
          tac._frames.push_back(f);
          //cout << tabs.c_str() << f << endl;
        }

        start_node = frame_node;
      } while (frame_node);

      tracks.push_back(tac);
    }

    start_node = curve_node;
  } while (curve_node);

  //ok - pack that sweet sweet data into our game format
  BoneAnim bone_animation;
  for (uint32_t i = 0; i < bone_names.size(); i++)
  {
    BoneAnimTrack bat;
    uint32_t bone_name_hash_id = Math::hash_value_from_string(bone_names[i].c_str());
    bat._bone = (Bone *)bone_name_hash_id;

    for (uint32_t j = 0; j < tracks.size(); j++)
    {
      if (tracks[j]._bone_name.compare(bone_names[i]) == 0)
      {
        switch (tracks[j]._transform_type)
        {
        case BONE_TRANSFORM_LOC:
          if (tracks[j]._frames.size() > bat._pos_frames.size())
          {
            bat._pos_frames.resize(tracks[j]._frames.size());
          }
          for (uint32_t k = 0; k < tracks[j]._frames.size(); k++)
          {
            uint32_t axis = (uint32_t)(tracks[j]._transform_axis);
            bat._pos_frames[k]._x = tracks[j]._frames[k][0];
            bat._pos_frames[k]._pos[axis] = tracks[j]._frames[k][1];
          }
          break;
        case BONE_TRANSFORM_ROT_QUAT:
          if (tracks[j]._frames.size() > bat._rot_frames.size())
          {
            bat._rot_frames.resize(tracks[j]._frames.size());
          }
          for (uint32_t k = 0; k < tracks[j]._frames.size(); k++)
          {
            uint32_t axis = (uint32_t)(tracks[j]._transform_axis);
            bat._rot_frames[k]._x = tracks[j]._frames[k][0];
            bat._rot_frames[k]._rot.q[axis] = tracks[j]._frames[k][1];
          }
          break;
        case BONE_TRANSFORM_SCALE:
          if (tracks[j]._frames.size() > bat._scale_frames.size())
          {
            bat._scale_frames.resize(tracks[j]._frames.size());
          }
          for (uint32_t k = 0; k < tracks[j]._frames.size(); k++)
          {
            uint32_t axis = (uint32_t)(tracks[j]._transform_axis);
            bat._scale_frames[k]._x = tracks[j]._frames[k][0];
            bat._scale_frames[k]._scale[axis] = tracks[j]._frames[k][1];
          }
          break;
        default:
          assert(false);  //something's wrong!
          break;
        }
      }
    }
    bone_animation.add_track(bat);
  }

  cout << tabs.c_str() << "unique bone names: " << endl;
  for (uint32_t i = 0; i < bone_names.size(); i++)
  {
    cout << tabs.c_str() << "\t" << bone_names[i].c_str() << endl;
  }

  //TODO: optimize frames based on curvature of the animation curve
  //      load in a curve file that represents slope thresholds over time
  //      if the curvature at a frame is less than the threshold, elminate that frame

  // ok, sweet - now we have our data in game format, more or less...
  // blast it into a .bin file.
  cout << tabs.c_str() << "opening file " << output_fname.c_str() << "..." << endl;
  FILE *f = fopen(output_fname.c_str(), "wb");
  assert(f);

  version = 1;
  cout << tabs.c_str() << "file version: " << version << endl;
  fwrite(&version, sizeof(uint32_t), 1, f);

  uint32_t num_tracks = bone_animation.get_num_tracks();
  fwrite(&num_tracks, sizeof(uint32_t), 1, f);

  for (uint32_t i = 0; i < num_tracks; i++)
  {
    BoneAnimTrack *bat = bone_animation.get_anim_track(i);

    uint32_t bone_hash_id = (uint32_t)bat->_bone;
    fwrite(&bone_hash_id, sizeof(uint32_t), 1, f);

    uint32_t num_pos_frames = bat->_pos_frames.size();
    uint32_t num_rot_frames = bat->_pos_frames.size();
    uint32_t num_scale_frames = bat->_pos_frames.size();

    fwrite(&num_pos_frames, sizeof(uint32_t), 1, f);
    fwrite(&num_rot_frames, sizeof(uint32_t), 1, f);
    fwrite(&num_scale_frames, sizeof(uint32_t), 1, f);

    fwrite(bat->_pos_frames.data(), sizeof(BoneTransformPos), num_pos_frames, f);
    fwrite(bat->_rot_frames.data(), sizeof(BoneTransformRot), num_rot_frames, f);
    fwrite(bat->_scale_frames.data(), sizeof(BoneTransformScale), num_scale_frames, f);
  }

  fclose(f);
}