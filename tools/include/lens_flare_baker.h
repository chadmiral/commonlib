#pragma once

#include "package_asset.h"
#include "tool.h"
#include "skeleton.h"

namespace Tool
{
  class TmpLensFlareElement
  {
  public:
    TmpLensFlareElement() {}
    ~TmpLensFlareElement() {}

    std::string     _name;
    std::string     _material;
  };

  class TmpLensFlare
  {
  public:
    TmpLensFlare() {}
    ~TmpLensFlare() {}

    void write_to_file(FILE *f);
    void read_from_file(FILE *f);

    uint32_t                          _version;
    std::string                       _name;

    Math::Float3                      _center;

    float                             _occlusion_radius;
    std::string                       _occlusion_mesh;

    std::vector<TmpLensFlareElement>  _elements;
  private:
    uint32_t calculate_element_file_size();
  };

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
    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "", std::ostream &log = std::cout);
    void load_xml(mxml_node_t *tree, TmpLensFlare &tmp_lf, std::string tabs, std::ostream &log);
  };
};
