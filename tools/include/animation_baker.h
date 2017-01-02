#pragma once

#include "tool.h"
#include "package_asset.h"
#include "skeleton.h"

namespace Tool
{
  class AnimationPackageAsset : public PackageAsset
  {
  public:
    AnimationPackageAsset() : PackageAsset(PACKAGE_ASSET_ANIMATION) {  }
    ~AnimationPackageAsset() { }

    Animation::SkeletonAnimation anim;
  };

  class AnimationBaker
  {
  public:
    AnimationBaker() {}
    ~AnimationBaker() {}

    void init() {}
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
  };
};