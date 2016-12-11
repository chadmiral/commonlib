#ifndef __STATIC_MESH_BAKER_H__
#define __STATIC_MESH_BAKER_H__

#include <vector>

#include "tool.h"
#include "draw_call.h"
#include "static_mesh.h"

namespace Tool
{
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

    void bake(mxml_node_t *tree, std::string output_fname);
  };
};

#endif //__STATIC_MESH_BAKER_H__
