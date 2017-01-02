#ifndef __STATIC_MESH_BAKER_H__
#define __STATIC_MESH_BAKER_H__

#include <vector>

#include "package_asset.h"
#include "tool.h"
#include "draw_call.h"
#include "static_mesh.h"

namespace Tool
{
  class MeshPackageAsset : public PackageAsset
  {
  public:
    MeshPackageAsset() : PackageAsset(PACKAGE_ASSET_MESH) { num_verts = 0; vertices = NULL; num_indices = 0; indices = NULL; }
    ~MeshPackageAsset() {}

    uint32_t num_verts;
    void *vertices;
    uint32_t num_indices;
    void *indices;
  };

  class MeshFace
  {
  public:
    Math::Float3 normal;
    //Float2 uvs[2];

    unsigned int mat_idx;
    unsigned int vert_idx[3];
    Math::Float3 rgb[3];
    Math::Float2 uvs[3];
  };

  class StaticMeshBaker
  {
  public:
    StaticMeshBaker() {}
    ~StaticMeshBaker() {}

    void init() {}

    void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
  };
};

#endif //__STATIC_MESH_BAKER_H__
