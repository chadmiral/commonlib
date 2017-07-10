#include <iostream>
#include <assert.h>

#include "tool.h"
#include "static_mesh_baker.h"
#include "kdtree.h"

using namespace Tool;
using namespace Graphics;
using namespace Math;
using namespace std;

#define VERTEX_MERGE_THRESHOLD 0.001f //TODO: make this user-specified?

bool verts_equal(StaticMeshVertex &a, StaticMeshVertex &b)
{
  float d = (a.x - b.x) * (a.x - b.x);
  d += (a.y - b.y) * (a.y - b.y);
  d += (a.z - b.z) * (a.z - b.z);

  d += (a.r - b.r) * (a.r - b.r);
  d += (a.g - b.g) * (a.r - b.g);
  d += (a.b - b.b) * (a.b - b.b);

  d += (a.nx - b.nx) * (a.nx - b.nx);
  d += (a.ny - b.ny) * (a.ny - b.ny);
  d += (a.nz - b.nz) * (a.nz - b.nz);

  d += (a.u0 - b.u0) * (a.u0 - b.u0);
  d += (a.v0 - b.v0) * (a.v0 - b.v0);

  return (d < VERTEX_MERGE_THRESHOLD);
}

void StaticMeshBaker::bake(mxml_node_t *tree, std::string output_fname, std::ostream &log, std::string tabs)
{
  tabs = tabs + "\t";
  log << tabs.c_str() << "Parsing static mesh xml..." << endl;

  //read all the materials
  mxml_node_t *material_node = NULL;
  mxml_node_t *start_node = tree;
  do
  {
    material_node = mxmlFindElement(start_node, tree, "material", NULL, NULL, MXML_DESCEND);

    if(material_node)
    {
      mxml_node_t *diff_node = mxmlFindElement(material_node, material_node, "diffuse_color", NULL, NULL, MXML_DESCEND);
      assert(diff_node);
      Float3 diff_color = mxml_read_float3(diff_node->child);
    }

    //read all the textures used by this material
    mxml_node_t *tex_node = NULL;
    mxml_node_t *t_start_node = material_node;

    do
    {
      tex_node = mxmlFindElement(t_start_node, material_node, "texture", NULL, NULL, MXML_DESCEND);
      if(tex_node)
      {
        //TODO
      }
      t_start_node = tex_node;
    } while(tex_node);

    start_node = material_node;
  } while(material_node);

  //read all the vertices
  std::vector<Float3> vertex_xyz;
  std::vector<Float3> vertex_normal;
  mxml_node_t *vert_node = NULL;
  start_node = tree;
  do
  {
    vert_node = mxmlFindElement(start_node, tree, "vertex", NULL, NULL, MXML_DESCEND);
    if(vert_node)
    {
      mxml_node_t *pos_node = mxmlFindElement(vert_node, vert_node, "vert_pos", NULL, NULL, MXML_DESCEND);
      assert(pos_node);
      Float3 vert_pos = mxml_read_float3(pos_node->child);
      vertex_xyz.push_back(vert_pos);

      mxml_node_t *norm_node = mxmlFindElement(vert_node, vert_node, "normal", NULL, NULL, MXML_DESCEND);
      assert(norm_node);
      Float3 vert_normal = mxml_read_float3(norm_node->child);
      vertex_normal.push_back(vert_normal);

      start_node = vert_node;
    }
  } while(vert_node);

  //read all the faces
  std::vector<MeshFace> mesh_faces;
  mxml_node_t *face_node = NULL;
  start_node = tree;
  do
  {
    face_node = mxmlFindElement(start_node, tree, "face", NULL, NULL, MXML_DESCEND);
    if(face_node)
    {
      MeshFace mf;

      mxml_node_t *face_idx_node = mxmlFindElement(face_node, face_node, "face_idx", NULL, NULL, MXML_DESCEND);
      assert(face_idx_node);

      int face_idx = atoi(face_idx_node->child->value.text.string);
      //cout<<"\tface id: "<<face_idx<<endl;

      mxml_node_t *mat_idx_node = mxmlFindElement(face_node, face_node, "mat_idx", NULL, NULL, MXML_DESCEND);
      mf.mat_idx = atoi(mat_idx_node->child->value.text.string);
      //cout<<"\t\tmat_idx: "<<mf.mat_idx<<endl;

      int v_idx[3] = { -1, -1, -1 };
      Float3 col[3];
      Float2 uvs[3];

      mxml_node_t *vidx_node = face_node;
      mxml_node_t *uv_node = face_node;
      mxml_node_t *col_node = face_node;
      for(int i = 0; i < 3; i++)
      {
        //uvs
        uv_node = mxmlFindElement(uv_node, face_node, "uv", NULL, NULL, MXML_DESCEND);
        assert(uv_node);

        mf.uvs[i] = mxml_read_float2(uv_node->child);
        //cout<<"\t\tuv: "<<mf.uvs[i]<<endl;

        //vertex color
        col_node = mxmlFindElement(col_node, face_node, "col", NULL, NULL, MXML_DESCEND);
        assert(col_node);
        mf.rgb[i] = mxml_read_float3(col_node->child);
        //cout<<"\t\trgb: "<<mf.rgb[i]<<endl;

        //vertex index
        vidx_node = mxmlFindElement(vidx_node, face_node, "v_idx", NULL, NULL, MXML_DESCEND);
        assert(vidx_node);
        mf.vert_idx[i] = atoi(vidx_node->child->value.text.string);
        //cout<<"\t\tvidx: "<<mf.vert_idx[i]<<endl;
      }

      //face normal
      mxml_node_t *norm_node = mxmlFindElement(face_node, face_node, "normal", NULL, NULL, MXML_DESCEND);
      assert(norm_node);
      mf.normal = mxml_read_float3(norm_node->child);
      mf.normal.normalize();
      //cout<<"\tfn: "<<mf.normal<<endl;

      mesh_faces.push_back(mf);

      start_node = face_node;
    }
  } while(face_node);

  //build render data
  std::vector<uint32_t> indices;
  std::vector<StaticMeshVertex> render_verts;
  int rvi = 0;

  Structures::KDTree3D<int> vert_tree;

  for(uint32_t i = 0; i < mesh_faces.size(); i++)
  {
    MeshFace *mf = &mesh_faces[i];
    for(int j = 0; j < 3; j++)
    {
      uint32_t vert_idx = mf->vert_idx[j];

      StaticMeshVertex smv;

      smv.x = vertex_xyz[vert_idx][0];
      smv.y = vertex_xyz[vert_idx][1];
      smv.z = vertex_xyz[vert_idx][2];

      smv.r = mf->rgb[j][0];
      smv.g = mf->rgb[j][1];
      smv.b = mf->rgb[j][2];

      smv.nx = vertex_normal[vert_idx][0];//mf->normal[0];
      smv.ny = vertex_normal[vert_idx][1];//mf->normal[1];
      smv.nz = vertex_normal[vert_idx][2];//mf->normal[2];

      smv.u0 = mf->uvs[j][0];
      smv.v0 = mf->uvs[j][1];

      bool found_twin = false;
      int indices_idx = render_verts.size();

      //see if we can find a duplicate vert and weld it
      Structures::KDNode<int> *closest = vert_tree.find_nearest_neighbor(Float3(smv.x, smv.y, smv.z));
      if (closest)
      {
        if (verts_equal(smv, render_verts[closest->data]))
        {
          indices_idx = closest->data;
          found_twin = true;
        }
      }

      if(!found_twin)
      {
        vert_tree.insert_element(Float3(smv.x, smv.y, smv.z), render_verts.size());
        render_verts.push_back(smv);
      }
      indices.push_back(indices_idx);
    }
  }

  log << tabs.c_str() << "opening file " << output_fname.c_str() << "..." << endl;
  FILE *f = fopen(output_fname.c_str(), "wb");
  assert(f);

  int version = 1;
  log << tabs.c_str() << "file version: " << version << endl;
  fwrite(&version, sizeof(int), 1, f);

  //write vertex data
  int num_render_verts = render_verts.size();
  log << tabs.c_str() << "writing " << num_render_verts << " render verts"<<endl;
  fwrite(&num_render_verts, sizeof(uint32_t), 1, f);
  fwrite(render_verts.data(), sizeof(StaticMeshVertex), num_render_verts, f);

  //write index data
  int num_indices = indices.size();
  log << tabs.c_str() << "writing " << num_indices << " indices" << endl;
  fwrite(&num_indices, sizeof(int), 1, f);
  fwrite(indices.data(), sizeof(unsigned int), num_indices, f);

  //DrawCall<StaticMeshVertex> dc;
  //dc.num_indices = mesh_faces.size();
  //dc.indices = indices;
  //dc.vertex_data = render_verts;

  fclose(f);
}
