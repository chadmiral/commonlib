#include <iostream>

#include "skeleton_baker.h"
#include "skeleton.h"

using namespace Tool;
using namespace std;
using namespace Math;

void SkeletonBaker::bake(mxml_node_t *tree, std::string output_fname)
{
  const char *buffer = NULL;

  cout << "\tParsing skeleton xml..." << endl;

  //read all the skeletons
  mxml_node_t *skeleton_node = NULL;
  mxml_node_t *start_node = tree; 

  std::vector<Animation::Bone> bones;
  std::vector<std::string>     bone_names;
  std::vector<std::string>     bone_parent_names;

  uint32_t num_skeletons = 0;

  do
  {
    skeleton_node = mxmlFindElement(start_node, tree, "skeleton", NULL, NULL, MXML_DESCEND);

    if (skeleton_node)
    {
      buffer = mxmlElementGetAttr(skeleton_node, "name");
      cout << "Skeleton Name: " << buffer << endl;

      num_skeletons++;
      assert(num_skeletons == 1); //TODO: allow multiple skeletons per rig?

      mxml_node_t *bone_node = NULL;
      start_node = skeleton_node;
      do
      {
        bone_node = mxmlFindElement(start_node, tree, "bone", NULL, NULL, MXML_DESCEND);
        if (bone_node)
        {
          Animation::Bone new_bone;
          new_bone._parent_idx = -1;

          //retrieve bone name
          buffer = mxmlElementGetAttr(bone_node, "name");
          cout << "Bone: " << buffer << endl;
          bone_names.push_back(buffer);

          //retrieve bone parent (if it exists)
          buffer = mxmlElementGetAttr(bone_node, "parent");
          if (buffer)
          {
            cout << "parent: " << buffer << endl;
            bone_parent_names.push_back(buffer);
          }
          else
          {
            bone_parent_names.push_back("UnlikelyBoneName777");
          }

          //retrieve head position
          mxml_node_t *head_node = mxmlFindElement(bone_node, tree, "head", NULL, NULL, MXML_DESCEND);
          assert(head_node);
          Float3 head_pos = mxml_read_float3(head_node->child);
          new_bone._head_pos = head_pos;
          cout << "head pos: " << head_pos << endl;

          //retrieve tail position
          mxml_node_t *tail_node = mxmlFindElement(bone_node, tree, "tail", NULL, NULL, MXML_DESCEND);
          assert(tail_node);
          Float3 tail_pos = mxml_read_float3(tail_node->child);
          new_bone._tail_pos = tail_pos;
          cout << "tail pos: " << tail_pos << endl;

          bones.push_back(new_bone);
        }
        start_node = bone_node;
      } while (bone_node);
    }

    start_node = skeleton_node;
  } while (skeleton_node);

  for (uint32_t i = 0; i < bone_parent_names.size(); i++)
  {
    int32_t parent_idx = -1;
    for (uint32_t j = 0; j < bone_names.size(); j++)
    {
      if (bone_names[j] == bone_parent_names[i])
      {
        parent_idx = j;
        break;
      }
    }
    bones[i]._parent_idx = parent_idx;
  }

  //now write the data to file
  cout << "opening file " << output_fname.c_str() << "..." << endl;
  FILE *f = fopen(output_fname.c_str(), "wb");
  assert(f);

  int version = 1;
  cout << "file version: " << version << endl;
  fwrite(&version, sizeof(int), 1, f);

  uint32_t num_bones = bones.size();
  fwrite(&num_skeletons, sizeof(uint32_t), 1, f);
  fwrite(&num_bones, sizeof(uint32_t), 1, f);
  fwrite(bones.data(), sizeof(Animation::Bone), num_bones, f);

  fclose(f);
}