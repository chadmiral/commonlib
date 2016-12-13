#pragma once

#include <vector>
#include "vector.h"
#include "quaternion.h"

namespace Animation
{
  class BoneAnim;

  struct Bone
  {
    int32_t         _parent_idx; //-1 for root
    Math::Float3    _head_pos;
    Math::Float3    _tail_pos;
  };

  typedef std::pair<std::string, Bone> BonePair;
  typedef std::vector<BonePair> BoneList;

  class Skeleton
  {
  private:
    std::string   _name;
    BoneList      _bones; //name, bone
  public:
    Skeleton() {}
    ~Skeleton() {}

    void set_name(std::string n) { _name = n; }
    uint32_t get_num_bones() const { return _bones.size(); }
    Bone *get_bone(const uint32_t idx) { return &(_bones[idx].second); }

    void add_bone(std::string name, Bone *bone) { _bones.push_back(BonePair(name, *bone)); }

    void transform(const float t, BoneAnim *anim);
    void render_debug();
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
    BONE_TRANSFORM_AXIS_X,
    BONE_TRANSFORM_AXIS_Y,
    BONE_TRANSFORM_AXIS_Z,
    BONE_TRANSFORM_AXIS_W,

    BONE_TRANSFORM_AXIS_INVALID,
    NUM_BONE_TRANSFORM_AXES
  };

  class BoneAnimTrack
  {
    friend class BoneAnim;
  private:
    Bone                             *_bone;
    std::vector<BoneTransformPos>     _pos_frames;
    std::vector<BoneTransformRot>     _rot_frames;
    std::vector<BoneTransformScale>   _scale_frames;
  public:
    BoneAnimTrack() {}
    ~BoneAnimTrack() {}

    void evaluate(const float x, BoneTransform *result);
  };

  class BoneAnim
  {
  private:
    std::vector<BoneAnimTrack> _tracks;
  public:
    BoneAnim() {}
    ~BoneAnim() {}
  };
};