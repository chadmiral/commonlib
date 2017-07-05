#pragma once

#include <vector>

namespace Tool
{
  enum PackageAssetType
  {
    PACKAGE_ASSET_SHADER,
    PACKAGE_ASSET_MATERIAL,
    PACKAGE_ASSET_TEXTURE,
    PACKAGE_ASSET_MESH,
    PACKAGE_ASSET_SKELETON,
    PACKAGE_ASSET_ANIMATION,
    PACKAGE_ASSET_UI_LAYOUT,
    PACKAGE_ASSET_LENS_FLARE,

    NUM_PACKAGE_ASSET_TYPES,
    INVALID_PACKAGE_ASSET
  };

  static const char *Package_asset_names[] =
  {
    "Shader",
    "Material",
    "Texture",
    "Mesh",
    "Skeleton",
    "Animation",
    "UI Layout",
    "Lens Flare",
  };

  class PackageAsset
  {
    friend class PackageBaker;
  protected:
    std::string name;
    std::string fname;
    PackageAssetType type;
    std::vector<std::string> *path;
  public:
    PackageAsset() {}
    PackageAsset(PackageAssetType t) { type = t; }
    ~PackageAsset() {}

    void set_name(const char *n) { name = n; }
    std::string get_name() const { return name; }

    void set_path(std::vector<std::string> *p) { path = p; }

    void set_type(PackageAssetType t) { type = t; }
    PackageAssetType get_type() const { return type; }
  };
};
