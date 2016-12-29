#pragma once

#include <stdio.h>
#include <vector>
#include "vector.h"
#include "quaternion.h"

namespace Animation
{
  class SkeletonAnimation;

  struct Bone
  {
    uint32_t        _hash_id;
    int32_t         _parent_idx; //-1 for root
    Math::Float3    _head_pos;
    Math::Float3    _tail_pos;
  };

  typedef std::vector<Bone> BoneList;

  class Skeleton
  {
    friend class SkeletonAnimation;
  private:
    std::string   _name;
    BoneList      _bones; //name, bone
  public:
    Skeleton() {}
    ~Skeleton() {}

    void set_name(std::string n) { _name = n; }
    uint32_t get_num_bones() const { return _bones.size(); }
    Bone *get_bone(const uint32_t idx) { return &(_bones[idx]); }

    void add_bone(Bone *bone)
    {
      _bones.push_back(*bone);
    }

    void transform(const float t, SkeletonAnimation *anim);
    void render_debug(SkeletonAnimation *animation = NULL, float anim_pct = 0.0f);
  };

  //typedef std::pair<Math::Quaternion, Math::Float3> BoneTransform;
  struct BoneTransformPos
  {
    float             _x; //curve x-axis (time)
    Math::Float3      _pos;
  };

  struct BoneTransformRot
  {
    float             _x; //curve x-axis (time)
    Math::Quaternion  _rot;
  };

  struct BoneTransformScale
  {
    float             _x; //curve x-axis (time)
    Math::Float3      _scale;
  };

  struct BoneTransform
  {
    BoneTransformPos   _pos;
    BoneTransformRot   _rot;
    BoneTransformScale _scale;
  };

  enum BoneTransformType
  {
    BONE_TRANSFORM_INVALID,
    BONE_TRANSFORM_ROT_QUAT,
    BONE_TRANSFORM_LOC,
    BONE_TRANSFORM_SCALE,

    NUM_BONE_TRANSFORM_TYPES
  };

  enum BoneTransformAxis
  {
    BONE_TRANSFORM_AXIS_INVALID = -1,
    BONE_TRANSFORM_AXIS_X = 0,
    BONE_TRANSFORM_AXIS_Y = 1,
    BONE_TRANSFORM_AXIS_Z = 2,
    BONE_TRANSFORM_AXIS_W = 3,

    NUM_BONE_TRANSFORM_AXES = 4
  };

  class BoneAnimTrack
  {
   // friend class SkeletonAnimation;
  //private:
  public:
    Bone    *_bone;
    uint32_t _bone_id;

    std::vector<BoneTransformPos>     _pos_frames;
    std::vector<BoneTransformRot>     _rot_frames;
    std::vector<BoneTransformScale>   _scale_frames;
  public:
    BoneAnimTrack() { _bone = NULL; _bone_id = 666; }
    ~BoneAnimTrack() {}

    void evaluate(const float x, BoneTransform *result)
    {
      //TODO: optimize the shit out of this
      for (uint32_t i = 0; i < _pos_frames.size(); i++)
      {
        if (_pos_frames[i]._x > x)
        {
          result->_pos = _pos_frames[i];
          break;
        }
      }
      for (uint32_t i = 0; i < _rot_frames.size(); i++)
      {
        if (_rot_frames[i]._x > x)
        {
          result->_rot = _rot_frames[i];
        }
      }
      for (uint32_t i = 0; i < _scale_frames.size(); i++)
      {
        if (_scale_frames[i]._x > x)
        {
          result->_scale = _scale_frames[i];
        }
      }
    }
  };

  class SkeletonAnimation
  {
  //friend class PackageBaker;
  private:
    uint32_t                   _num_frames;
    uint16_t                   _frame_rate_fps;
  public:
    std::vector<BoneAnimTrack> _tracks;
    
    SkeletonAnimation() {}
    ~SkeletonAnimation() {}

    void read_from_file(FILE *f);
    uint8_t *read_from_memory(uint8_t *data_ptr, uint32_t num_tracks);

    void bind(Skeleton *s)
    {
      for (uint32_t i = 0; i < s->_bones.size(); i++)
      {
        Bone *b = &s->_bones[i];
        for (uint32_t j = 0; j < _tracks.size(); j++)
        {
          if (b->_hash_id == (uint32_t)_tracks[j]._bone_id)
          {
            _tracks[j]._bone = b;
          }
        }
      }
    }

    uint32_t get_num_tracks() const { return _tracks.size(); }
    BoneAnimTrack *get_anim_track(const uint32_t idx) { return &_tracks[idx]; }
  };
};
