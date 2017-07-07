#pragma once

#include "package_asset.h"
#include "tool.h"
#include "skeleton.h"

namespace Tool
{
  struct SkeletonTemplate
  {
    std::string _name;
    std::string _fname;
  };

  class SkeletonPackageAsset : public PackageAsset
  {
  public:
    SkeletonPackageAsset() : PackageAsset(PACKAGE_ASSET_SKELETON) { num_bones = 0; bones = NULL; }
    SkeletonPackageAsset(SkeletonTemplate &st) : PackageAsset(PACKAGE_ASSET_SKELETON)
    {
      name = st._name;
      fname = st._fname;
    }
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
    void bake(mxml_node_t *tree, std::string output_fname, std::ostream &log = std::cout, std::string tabs = "");
  };
};