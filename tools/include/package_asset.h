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
    PACKAGE_ASSET_UI_LAYOUT
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