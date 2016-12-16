#pragma once

#include "tool.h"

namespace Tool
{
  class AnimationBaker
  {
  public:
    AnimationBaker() {}
    ~AnimationBaker() {}

    void init() {}
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
  };
};