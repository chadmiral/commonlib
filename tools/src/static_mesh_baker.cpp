#include <iostream>
#include <assert.h>

#include "tool.h"
#include "static_mesh_baker.h"

using namespace Tool;
using namespace Graphics;
using namespace Math;
using namespace std;

StaticMeshBaker::StaticMeshBaker()
{

}

void StaticMeshBaker::bake(mxml_node_t *tree, std::string output_fname)
{
  cout<<"\tParsing static mesh xml..."<<endl;

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
      //cout<<"\tdiffuse: "<<diff_color<<endl;
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
      //cout<<"\tv: "<<vert_pos<<endl;

      mxml_node_t *norm_node = mxmlFindElement(vert_node, vert_node, "normal", NULL, NULL, MXML_DESCEND);
      assert(norm_node);
      Float3 vert_normal = mxml_read_float3(norm_node->child);
      //cout<<"\tn: "<<vert_normal<<endl;

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
  //cout<<endl<<endl<<"\t"<<mesh_faces.size()<<" faces."<<endl;
  uint32_t num_indices = mesh_faces.size() * 3;
  unsigned int *indices = new unsigned int[num_indices];
  int index_counter = 0;

  uint32_t num_render_verts = mesh_faces.size() * 3;
  StaticMeshVertex *render_verts = new StaticMeshVertex[num_render_verts];
  int rvi = 0;

  for(unsigned int i = 0; i < mesh_faces.size(); i++)
  {
    MeshFace *mf = &mesh_faces[i];
    //cout<<"\tvert_idx:\n";
    for(int j = 0; j < 3; j++)
    {
      unsigned int vert_idx = mf->vert_idx[j];
      indices[index_counter] = index_counter;//vert_idx;
      index_counter++;

      //cout<<"\t\t"<<mf->vert_idx[j]<<" ";
      //cout<<vertex_xyz[i]<<endl;

      render_verts[rvi].x = vertex_xyz[vert_idx][0];
      render_verts[rvi].y = vertex_xyz[vert_idx][1];
      render_verts[rvi].z = vertex_xyz[vert_idx][2];

      render_verts[rvi].r = mf->rgb[j][0];
      render_verts[rvi].g = mf->rgb[j][1];
      render_verts[rvi].b = mf->rgb[j][2];

      render_verts[rvi].nx = mf->normal[0];
      render_verts[rvi].ny = mf->normal[1];
      render_verts[rvi].nz = mf->normal[2];

      render_verts[rvi].u0 = mf->uvs[j][0];
      render_verts[rvi].v0 = mf->uvs[j][1];

      rvi++;
    }
    //cout<<endl;
  }

  cout<<"opening file "<<output_fname.c_str()<<"..."<<endl;
  FILE *f = fopen(output_fname.c_str(), "wb");
  assert(f);

  int version = 1;
  cout<<"file version: "<<version<<endl;
  fwrite(&version, sizeof(int), 1, f);

  //write vertex data
  cout<<"writing "<<num_render_verts<<" render verts"<<endl;
  fwrite(&num_render_verts, sizeof(uint32_t), 1, f);
  fwrite(render_verts, sizeof(StaticMeshVertex), num_render_verts, f);

  //write index data
  cout<<"writing "<<num_indices<<" indices"<<endl;
  fwrite(&num_indices, sizeof(int), 1, f);
  fwrite(indices, sizeof(unsigned int), num_indices, f);

  //DrawCall<StaticMeshVertex> dc;
  //dc.num_indices = mesh_faces.size();
  //dc.indices = indices;
  //dc.vertex_data = render_verts;

  fclose(f);

  delete indices;
  delete render_verts;
}
