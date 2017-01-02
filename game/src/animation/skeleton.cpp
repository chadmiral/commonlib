#include <iostream>
#include "skeleton.h"
#include "platform_gl.h"
#include "matrix.h"

using namespace Animation;
using namespace std;
using namespace Math;

void Skeleton::transform(const float t, SkeletonAnimation *a)
{
  cout << "Skeleton::transform(): " << _name.c_str() << endl;
}

void Skeleton::render_debug(SkeletonAnimation *animation, float anim_pct)
{
#ifndef __LOKI__

  //glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
  //for (uint32_t i = 0; i < _bones.size(); i++)
  uint32_t i = 0;
  {
    if (_bones[i]._parent_idx == -1)
    {
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
      glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    }

    BoneTransform bt;
    Matrix3x3 rotation;

    if (animation)
    {
      //find the track that matches this bone
      for (uint32_t j = 0; j < animation->_tracks.size(); j++)
      {
        if (animation->_tracks[j]._bone == &_bones[i])
        {
          animation->_tracks[j].evaluate(anim_pct, &bt);
          break;
        }
      }
    }

    rotation.rotation_from_quaternion(bt._rot._rot);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    rotation.transpose();

    GLfloat gl_mat[16] =
    {
      rotation(0, 0), rotation(1, 0), rotation(2, 0), 0.0f,
      rotation(0, 1), rotation(1, 1), rotation(2, 1), 0.0f,
      rotation(0, 2), rotation(1, 2), rotation(2, 2), 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    glMultMatrixf(gl_mat);
    {
      Float3 head = _bones[i]._head_pos;// + bt._pos._pos;
      Float3 tail = _bones[i]._tail_pos;// + bt._pos._pos;
      glDisable(GL_DEPTH_TEST);
      glBegin(GL_LINES);
        glVertex3f(head[0], head[2], head[1]);
        glVertex3f(tail[0], tail[2], tail[1]);
      glEnd();
    }
    glPopMatrix();
  }
#endif
}

void SkeletonAnimation::read_from_file(FILE *fp)
{
  int version;
  fread(&version, sizeof(int), 1, fp);

  uint32_t num_tracks;
  fread(&num_tracks, sizeof(uint32_t), 1, fp);

  //std::vector<BoneAnimTrack> tracks;
  for (uint32_t i = 0; i < num_tracks; i++)
  {
    BoneAnimTrack bat;

    //uint32_t hash_id;
    fread(&bat._bone_id, sizeof(uint32_t), 1, fp);

    uint32_t num_pos_frames, num_rot_frames, num_scale_frames;
    fread(&num_pos_frames, sizeof(uint32_t), 1, fp);
    fread(&num_rot_frames, sizeof(uint32_t), 1, fp);
    fread(&num_scale_frames, sizeof(uint32_t), 1, fp);

    bat._pos_frames.resize(num_pos_frames);
    bat._rot_frames.resize(num_rot_frames);
    bat._scale_frames.resize(num_scale_frames);

    fread(bat._pos_frames.data(), sizeof(BoneTransformPos), num_pos_frames, fp);
    fread(bat._rot_frames.data(), sizeof(BoneTransformRot), num_rot_frames, fp);
    fread(bat._scale_frames.data(), sizeof(BoneTransformScale), num_scale_frames, fp);

    _tracks.push_back(bat);
  }
}

uint8_t *SkeletonAnimation::read_from_memory(uint8_t *data_ptr, uint32_t num_tracks)
{
  for (uint32_t j = 0; j < num_tracks; j++)
  {
    BoneAnimTrack t;
    t._bone_id = ((uint32_t *)data_ptr)[0];
    data_ptr += sizeof(uint32_t); //bone pointer / hash id

    uint32_t num_pos_frames = ((uint32_t *)data_ptr)[0];
    uint32_t num_rot_frames = ((uint32_t *)data_ptr)[1];
    uint32_t num_scale_frames = ((uint32_t *)data_ptr)[2];

    data_ptr += 3 * sizeof(uint32_t); //skip past the 3 num_frames variables

    t._pos_frames.resize(num_pos_frames);
    memcpy(t._pos_frames.data(), data_ptr, sizeof(BoneTransformPos) * num_pos_frames);
    data_ptr += num_pos_frames * sizeof(BoneTransformPos);

    t._rot_frames.resize(num_rot_frames);
    memcpy(t._rot_frames.data(), data_ptr, sizeof(BoneTransformRot) * num_rot_frames);
    data_ptr += num_rot_frames * sizeof(BoneTransformRot);

    t._scale_frames.resize(num_scale_frames);
    memcpy(t._scale_frames.data(), data_ptr, sizeof(BoneTransformScale) * num_scale_frames);
    data_ptr += num_scale_frames * sizeof(BoneTransformScale);

    _tracks.push_back(t);
  }
  return data_ptr;
}