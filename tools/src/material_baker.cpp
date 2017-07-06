#include <iostream>
#include "material_baker.h"
#include "platform.h"

using namespace std;
using namespace Tool;

void TmpMaterial::write_to_file(FILE *f)
{
  uint16_t string_len = (uint16_t)_name.size() + 1;
  fwrite(&string_len, sizeof(uint16_t), 1, f);
  fwrite(_name.c_str(), sizeof(char), string_len, f);

  fwrite(&_vertex_size, sizeof(uint8_t), 1, f);

  string_len = (uint16_t)_target_buffer.size() + 1;
  fwrite(&string_len, sizeof(uint16_t), 1, f);
  fwrite(_target_buffer.c_str(), sizeof(char), string_len, f);

  string_len = (uint16_t)_shader_name.size() + 1;
  fwrite(&string_len, sizeof(uint16_t), 1, f);
  fwrite(_shader_name.c_str(), sizeof(char), string_len, f);

  fwrite(&_backface_cull, sizeof(bool), 1, f);
  fwrite(&_winding_order, sizeof(GLenum), 1, f);
  fwrite(&_blending, sizeof(bool), 1, f);
  fwrite(&_dst_blend_mode, sizeof(GLenum), 1, f);
  fwrite(&_src_blend_mode, sizeof(GLenum), 1, f);
  fwrite(&_depth_read, sizeof(bool), 1, f);
  fwrite(&_depth_write, sizeof(bool), 1, f);

  //write all the texture names
  uint16_t tex_count = _texture_names.size();
  fwrite(&tex_count, sizeof(uint16_t), 1, f);
  for (uint16_t i = 0; i < tex_count; i++)
  {
    string_len = _texture_names[i].size() + 1;
    fwrite(&string_len, sizeof(uint16_t), 1, f);
    fwrite(_texture_names[i].c_str(), sizeof(char), string_len, f);
  }

  //write all the shader uniforms
  uint16_t uniform_count = _uniforms.size();
  fwrite(&uniform_count, sizeof(uint16_t), 1, f);
  for (uint16_t i = 0; i < uniform_count; i++)
  {
    string_len = _uniforms[i]._name.size() + 1;
    fwrite(&string_len, sizeof(uint16_t), 1, f);
    fwrite(_uniforms[i]._name.c_str(), sizeof(char), string_len, f);

    uint32_t type = (uint32_t)_uniforms[i]._type;
    fwrite(&type, sizeof(uint32_t), 1, f);

    //TODO: allow for default values for float2, float3, etc...
    uint32_t num_def_values = DefaultUniformValueCounts[_uniforms[i]._type];
    fwrite(_uniforms[i]._val, sizeof(float), num_def_values, f);
  }

  //write all the vertex attribs 
  uint16_t attrib_count = _attribs.size();
  fwrite(&attrib_count, sizeof(uint16_t), 1, f);
  for (uint16_t i = 0; i < attrib_count; i++)
  {
    string_len = _attribs[i]._name.size() + 1;
    fwrite(&string_len, sizeof(uint16_t), 1, f);
    fwrite(_attribs[i]._name.c_str(), sizeof(char), string_len, f);

    fwrite(&_attribs[i]._offset, sizeof(uint32_t), 1, f);
    fwrite(&_attribs[i]._stride, sizeof(uint32_t), 1, f);
  }
}

void TmpMaterial::read_from_file(FILE *f)
{

}

GLenum string_to_blend_mode(const char *buffer)
{
  if (stricmp(buffer, "GL_ZERO") == 0)
  {
    return GL_ZERO;
  }
  else if (stricmp(buffer, "GL_ONE") == 0)
  {
    return GL_ONE;
  }
  else if (stricmp(buffer, "GL_SRC_ALPHA") == 0)
  {
    return GL_SRC_ALPHA;
  }
  else if (stricmp(buffer, "GL_ONE_MINUS_SRC_ALPHA") == 0)
  {
    return GL_ONE_MINUS_SRC_ALPHA;
  }
  assert(false);
  return GL_INVALID_ENUM;
}

void MaterialBaker::load_xml(mxml_node_t *tree, TmpMaterial &tmp_mat, std::string tabs, std::ostream &log)
{
  const char *buffer = NULL;

  log << tabs.c_str() << "Parsing material xml...";
  tabs += "\t";

  mxml_node_t *mat_node = mxmlFindElement(tree, tree, "material", NULL, NULL, MXML_DESCEND);
  assert(mat_node);

  buffer = mxmlElementGetAttr(mat_node, "version");
  uint32_t version = atoi(buffer);
  log << "version " << version << endl;

  buffer = mxmlElementGetAttr(mat_node, "name");
  tmp_mat._name = buffer;
  log << tabs.c_str() << "name: " << buffer << endl;

  buffer = mxmlElementGetAttr(mat_node, "vertex_size_bytes");
  tmp_mat._vertex_size = atoi(buffer);

  mxml_node_t *node = mxmlFindElement(mat_node, tree, "target_buffer", NULL, NULL, MXML_DESCEND);
  buffer = mxmlGetText(node, NULL);
  tmp_mat._target_buffer = buffer;

  //<shader>shader_name</shader>
  node = mxmlFindElement(mat_node, tree, "shader", NULL, NULL, MXML_DESCEND);
  buffer = mxmlGetText(node, NULL);
  tmp_mat._shader_name = buffer;

  //<alpha_blend>true</alpha_blend>
  node = mxmlFindElement(mat_node, tree, "alpha_blend", NULL, NULL, MXML_DESCEND);
  buffer = mxmlGetText(node, NULL);
  tmp_mat._blending = (stricmp(buffer, "true") == 0);
  buffer = mxmlElementGetAttr(node, "src_mode");
  tmp_mat._src_blend_mode = string_to_blend_mode(buffer);
  buffer = mxmlElementGetAttr(node, "dst_mode");
  tmp_mat._dst_blend_mode = string_to_blend_mode(buffer);

  //<depth_read>true</depth_read>
  node = mxmlFindElement(mat_node, tree, "depth_read", NULL, NULL, MXML_DESCEND);
  buffer = mxmlGetText(node, NULL);
  tmp_mat._depth_read = (stricmp(buffer, "true") == 0);

  //<depth_write>true</depth_write>
  node = mxmlFindElement(mat_node, tree, "depth_write", NULL, NULL, MXML_DESCEND);
  buffer = mxmlGetText(node, NULL);
  tmp_mat._depth_write = (stricmp(buffer, "true") == 0);

  //<backface_cull>true</backface_cull>
  node = mxmlFindElement(mat_node, tree, "backface_cull", NULL, NULL, MXML_DESCEND);
  buffer = mxmlGetText(node, NULL);
  tmp_mat._backface_cull = (stricmp(buffer, "true") == 0);
  buffer = mxmlElementGetAttr(node, "winding_order");
  if (stricmp(buffer, "ccw") == 0) { tmp_mat._winding_order = GL_CCW; }
  else if (stricmp(buffer, "cw") == 0) { tmp_mat._winding_order = GL_CW; }

  //<texture>texture_name< / texture>
  mxml_node_t *start_node = mat_node;
  do
  {
    mxml_node_t *tex_node = mxmlFindElement(start_node, tree, "texture", NULL, NULL, MXML_DESCEND);
    if (tex_node)
    {
      buffer = mxmlGetText(tex_node, NULL);
      std::string tex_name = buffer;
      tmp_mat._texture_names.push_back(tex_name);
    }
    start_node = tex_node;
  } while (start_node);

  //<uniform type = "uniform_type">uniform_name</uniform>
  start_node = mat_node;
  do
  {
    mxml_node_t *uniform_node = mxmlFindElement(start_node, tree, "uniform", NULL, NULL, MXML_DESCEND);
    if (uniform_node)
    {
      TmpUniform uni;
      buffer = mxmlElementGetAttr(uniform_node, "type");
      if (stricmp(buffer, "float") == 0)
      {
        uni._type = TMP_UNIFORM_FLOAT;
        uni._val = new float;
        buffer = mxmlGetText(uniform_node, NULL);
        uni._val[0] = atof(buffer);
      }
      else if (stricmp(buffer, "float2") == 0)
      {
        uni._type = TMP_UNIFORM_FLOAT2;
        uni._val = new float[2];
        buffer = mxmlGetText(uniform_node, NULL);
        memset(uni._val, 0, sizeof(float) * 2);
        if (uniform_node->child)
        {
          uni._val[0] = atof(uniform_node->child->value.text.string);
          uni._val[1] = atof(uniform_node->child->next->value.text.string);
        }
      }
      else if (stricmp(buffer, "float3") == 0)
      {
        uni._type = TMP_UNIFORM_FLOAT3;
        uni._val = new float[3];
        buffer = mxmlGetText(uniform_node, NULL);
        memset(uni._val, 0, sizeof(float) * 3);
        if (uniform_node->child)
        {
          uni._val[0] = atof(uniform_node->child->value.text.string);
          uni._val[1] = atof(uniform_node->child->next->value.text.string);
          uni._val[2] = atof(uniform_node->child->next->next->value.text.string);
        }
      }
      else if (stricmp(buffer, "float4") == 0)
      {
        uni._type = TMP_UNIFORM_FLOAT4;
        uni._val = new float[4];
        memset(uni._val, 0, sizeof(float) * 4);
        if (uniform_node->child)
        {
          uni._val[0] = atof(uniform_node->child->value.text.string);
          uni._val[1] = atof(uniform_node->child->next->value.text.string);
          uni._val[2] = atof(uniform_node->child->next->next->value.text.string);
          uni._val[3] = atof(uniform_node->child->next->next->next->value.text.string);
        }
      }
      else if (stricmp(buffer, "mat4x4") == 0)
      {
        uni._type = TMP_UNIFORM_MAT4X4;
      }
      else
      {
        log << "Error!: unknown uniform variable type!" << endl;
        assert(false);
      }
      buffer = mxmlElementGetAttr(uniform_node, "name");
      uni._name = buffer;
      tmp_mat._uniforms.push_back(uni);
    }
    start_node = uniform_node;
  } while (start_node);

  //<vertex_attrib offset = "0" stride = "3">in_xyz< / vertex_attrib>
  start_node = mat_node;
  do
  {
    mxml_node_t *attrib_node = mxmlFindElement(start_node, tree, "vertex_attrib", NULL, NULL, MXML_DESCEND);
    if (attrib_node)
    {
      TmpAttrib attr;
      buffer = mxmlElementGetAttr(attrib_node, "offset");
      attr._offset = atoi(buffer);
      buffer = mxmlElementGetAttr(attrib_node, "stride");
      attr._stride = atoi(buffer);
      buffer = mxmlGetText(attrib_node, NULL);
      attr._name = buffer;

      tmp_mat._attribs.push_back(attr);
    }
    start_node = attrib_node;
  } while (start_node);
}

void MaterialBaker::bake(mxml_node_t *tree, string output_fname, string tabs, std::ostream &log)
{
  tabs += "\t";

  TmpMaterial tmp_mat;
  load_xml(tree, tmp_mat, tabs, log);

  //ok, we've gathered all our data, now write to a binary file
  cout << tabs.c_str() << "opening file " << output_fname.c_str() << "..." << endl;
  FILE *f = fopen(output_fname.c_str(), "wb");
  assert(f);
  tmp_mat.write_to_file(f);
  fclose(f);
}