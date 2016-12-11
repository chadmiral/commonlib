#pragma once

#include <vector>
#include "vector.h"

namespace Animation
{
  class AnimAction
  {
  public:
    AnimAction() {}
    ~AnimAction() {}
  };

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

    void transform(const float t, AnimAction *anim);
    void render_debug();
  };
};