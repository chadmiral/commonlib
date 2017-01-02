#pragma once

#include "package_asset.h"
#include "tool.h"
#include "skeleton.h"

namespace Tool
{
  class SkeletonPackageAsset : public PackageAsset
  {
  public:
    SkeletonPackageAsset() : PackageAsset(PACKAGE_ASSET_SKELETON) { num_bones = 0; bones = NULL; }
    ~SkeletonPackageAsset() { if (bones) { delete bones; } }

    uint32_t            num_bones;
    Animation::Bone    *bones;
  };

  class SkeletonBaker
  {
  public:
    SkeletonBaker() {}
    ~SkeletonBaker() {}

    void init() {}
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
  };
};