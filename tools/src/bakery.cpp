#include <mxml.h>
#include <assert.h>

#include "bakery.h"

using namespace Tool;
using namespace std;
using namespace Math;

void Bakery::init()
{
  //init openGL / glew so we can compile shaders n shi

  package_baker.init();
  static_mesh_baker.init();
  skeleton_baker.init();
  bphys_baker.init();
  shader_baker.init();
}

void Bakery::bake(std::string fname, std::string out_fname)
{
  //extract the file extension
  //march backwards from the end of the string
  int dot_idx = fname.size() - 1;
  for(; fname[dot_idx] != '.'; dot_idx--) {}
  std::string fname_ext = fname.substr(dot_idx + 1, fname.size() - dot_idx);

  if(fname_ext == std::string("bphys"))
  {
    cout<<"Baking Blender physics file to texture..."<<endl;
    FILE *fp = fopen(fname.c_str(), "rb");
    if(fp)
    {
      bphys_baker.bake(fp, out_fname);
    }
    else
    {
      cerr<<"Bakery::bake() - Could not open file! "<<endl<<"\t"<<fname.c_str()<<endl;
    }
  }
  else
  {
    std::string output_fname = fname + ".bin";
    FILE *fp = fopen(fname.c_str(), "r");
    if(fp)
    {
      //unsigned int hash_id = hash(fname.c_str());
      //cout<<"\thash value:"<<hash_id<<endl;

      mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
      assert(tree);

      //don't need the file anymore now that we have the xml tree
      fclose(fp);

      mxml_node_t *node;

      //check if this is a static mesh
      node = mxmlFindElement(tree, tree, "static_mesh", "version", NULL, MXML_DESCEND);
      if(node)
      {
        static_mesh_baker.bake(tree, output_fname);
      }

      //shader
      node = mxmlFindElement(tree, tree, "shader_graph", "version", NULL, MXML_DESCEND);
      if(node)
      {
        shader_baker.bake(tree, output_fname);
      }

      //skeleton
      node = mxmlFindElement(tree, tree, "skeleton", "version", NULL, MXML_DESCEND);
      if (node)
      {
        skeleton_baker.bake(tree, output_fname);
      }

      //animation
      node = mxmlFindElement(tree, tree, "animation", "version", NULL, MXML_DESCEND);
      if (node)
      {
        animation_baker.bake(tree, output_fname);
      }

      //package
      node = mxmlFindElement(tree, tree, "package", "version", NULL, MXML_DESCEND);
      if (node)
      {
        PackageTemplate pt;
        pt._version = atoi(mxmlElementGetAttr(node, "version"));
        package_baker.bake(tree, output_fname, pt);
      }
    }
    else
    {
      cerr<<"Bakery::bake() - Could not open file! "<<endl<<"\t"<<fname.c_str()<<endl;
    }
  }
}
