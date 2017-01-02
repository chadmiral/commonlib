#ifndef __SHADER_BAKER_H__
#define __SHADER_BAKER_H__

#include "package_asset.h"

namespace Tool
{
  class ShaderPackageAsset : public PackageAsset
  {
  public:
    ShaderPackageAsset() : PackageAsset(PACKAGE_ASSET_SHADER) {}
    ~ShaderPackageAsset() {}

    std::string vs_fname;
    std::string fs_fname;

    std::string vs_source;
    std::string fs_source;

    std::string include_shader(std::string inc_fname);
    void parse_source(std::string source, std::string *dest);
  };

  class ShaderBaker
  {
    public:
      ShaderBaker();
      ~ShaderBaker();
      void init();
      void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
    private:
      void parse_shader_block(std::string fname);
  };
};

#endif //__SHADER_BAKER_H__
