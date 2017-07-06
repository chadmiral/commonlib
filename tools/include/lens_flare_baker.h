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
    LensFlarePackageAsset() : PackageAsset(PACKAGE_ASSET_LENS_FLARE)
    {
      _file_size = 0;
      _file_data = NULL;
    }

    LensFlarePackageAsset(BasicTemplate &mt) : PackageAsset(PACKAGE_ASSET_LENS_FLARE)
    {
      _file_size = 0;
      _file_data = NULL;

      name = mt._name;
      fname = mt._fname;
    }

    ~LensFlarePackageAsset()
    {
      if (_file_data) { delete _file_data; }
    }

    uint32_t _file_size;
    uint8_t *_file_data;
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
