#pragma once

#include "package_asset.h"
#include "tool.h"
#include "skeleton.h"

namespace Tool
{
  class LensFlarePackageAsset : public PackageAsset
  {
  public:
    LensFlarePackageAsset() : PackageAsset(PACKAGE_ASSET_LENS_FLARE) { }
    LensFlarePackageAsset(BasicTemplate &bt) : PackageAsset(PACKAGE_ASSET_LENS_FLARE)
    {
      name = bt._name;
      fname = bt._fname;
    }
    ~LensFlarePackageAsset() { if (bones) { delete bones; } }

    uint32_t            num_bones;
    Animation::Bone    *bones;
  };

  class LensFlareBaker
  {
  public:
    LensFlareBaker() {}
    ~LensFlareBaker() {}

    void init() {}
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
  };
};
