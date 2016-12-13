#pragma once

#include "tool.h"

namespace Tool
{
  class SkeletonBaker
  {
  public:
    SkeletonBaker() {}
    ~SkeletonBaker() {}

    void init() {}
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
  };
};