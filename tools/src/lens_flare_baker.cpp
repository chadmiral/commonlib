#include <iostream>

#include "tool.h"
#include "lens_flare_baker.h"
#include "platform.h"

using namespace std;
using namespace Tool;

void TmpLensFlare::write_to_file(FILE *f)
{
  fwrite(&_version, sizeof(uint32_t), 1, f);

  uint16_t str_len = _name.size() + 1;
  fwrite(&str_len, sizeof(uint16_t), 1, f);
  fwrite(_name.c_str(), sizeof(char), str_len, f);

  fwrite(_center._val, sizeof(float), 3, f);

  fwrite(&_occlusion_radius, sizeof(float), 1, f);
  str_len = _occlusion_mesh.size() + 1;
  fwrite(&str_len, sizeof(uint16_t), 1, f);
  fwrite(_occlusion_mesh.c_str(), sizeof(char), str_len, f);

  uint16_t num_elements = _elements.size();
  fwrite(&num_elements, sizeof(uint16_t), 1, f);

  //todo: calculate total size of all elements
  //fwrite(_elements.data(), )
}

void LensFlareBaker::load_xml(mxml_node_t *tree, TmpLensFlare &tlf, std::string tabs, std::ostream &log)
{
  const char *buffer = NULL;
  cout << tabs.c_str() << "Parsing lens flare xml..." << endl;

  mxml_node_t *lf_node = mxmlFindElement(tree, tree, "lens_flare", NULL, NULL, MXML_DESCEND);
  assert(lf_node);

  buffer = mxmlElementGetAttr(lf_node, "version");
  tlf._version = atoi(buffer);
  //log << tabs.c_str() << "version " << tlf._version << endl;

  buffer = mxmlElementGetAttr(lf_node, "name");
  tlf._name = buffer;
  log << tabs.c_str() << "name: " << buffer << endl;

  mxml_node_t *center_node = mxmlFindElement(lf_node, lf_node, "center", NULL, NULL, MXML_DESCEND);
  if(center_node)
  {
    tlf._center = mxml_read_float3(center_node->child);
    log << tabs.c_str() << "center: " << tlf._center << endl;
  }

  mxml_node_t *occlusion_radius_node = mxmlFindElement(lf_node, lf_node, "occlusion_radius", NULL, NULL, MXML_DESCEND);
  if(occlusion_radius_node)
  {
    buffer = mxmlGetText(occlusion_radius_node, NULL);
    tlf._occlusion_radius = atof(buffer);
    log << tabs.c_str() << "occlusion radius: " << tlf._occlusion_radius << endl;
  }

  mxml_node_t *occlusion_mesh_node = mxmlFindElement(lf_node, lf_node, "occlusion_mesh", NULL, NULL, MXML_DESCEND);
  if(occlusion_mesh_node)
  {
    tlf._occlusion_mesh = mxmlGetText(occlusion_mesh_node, NULL);
    log << tabs.c_str() << "occlusion mesh: "<< tlf._occlusion_mesh << endl;
  }

  //read in each lens flare element
  mxml_node_t *start_node = lf_node;
  do {
    mxml_node_t *lf_element_node = mxmlFindElement(start_node, lf_node, "element", NULL, NULL, MXML_DESCEND);

    TmpLensFlareElement lfe;

    buffer = mxmlElementGetAttr(lf_element_node, "name");
    if(buffer) { lfe._name = buffer; }
    buffer = mxmlElementGetAttr(lf_element_node, "material");
    if(buffer) { lfe._material = buffer; }

    //log << tabs.c_str() << "element : " << lfe._name << endl;
    //log << tabs.c_str() << "\tmaterial: " << lfe._material << endl;
    tlf._elements.push_back(lfe);

    start_node = lf_element_node;
  } while(start_node);

  log << tabs.c_str() << tlf._elements.size() << " lens flare elements..." << endl;
}

void LensFlareBaker::bake(mxml_node_t *tree, std::string output_fname, std::string tabs, std::ostream &log)
{
  tabs = tabs + "\t";

  TmpLensFlare tlf;

  load_xml(tree, tlf, tabs, log);

  //ok, we've gathered all our data, now write to a binary file
  cout << tabs.c_str() << "opening file " << output_fname.c_str() << "..." << endl;
  FILE *f = fopen(output_fname.c_str(), "wb");
  assert(f);
  tlf.write_to_file(f);
  fclose(f);
}
