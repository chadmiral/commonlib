#pragma once

#include "tool.h"
#include "package_asset.h"
#include "skeleton.h"

namespace Tool
{
  struct AnimationTemplate
  {
    std::string _name;
    std::string _fname;
  };

  class AnimationPackageAsset : public PackageAsset
  {
  public:
    AnimationPackageAsset() : PackageAsset(PACKAGE_ASSET_ANIMATION) {  }
    AnimationPackageAsset(AnimationTemplate &at) : PackageAsset(PACKAGE_ASSET_ANIMATION)
    {
      name = at._name;
      fname = at._fname;
    }
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