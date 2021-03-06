#include "platform_gl.h"
#include "platform_sdl.h"

#include <iostream>
#include <assert.h>
#include "tool.h"

#include "math_utility.h"
#include "platform.h"
#include "shader.h"
#include "skeleton.h"

#include "package_baker.h"
#include "static_mesh_baker.h"
#include "skeleton_baker.h"
#include "animation_baker.h"
#include "lens_flare_baker.h"

using namespace Math;
using namespace std;
using namespace Tool;
using namespace Graphics;
using namespace Animation;

/*
TODO: WIP - hoping to use this function to reduce a bunch of code in parse_xml
template<typename T>
void parse_assets(std::string name)
{
  //parse all the material assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, name.c_str(), NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      T tt;
      //parse_material_xml(asset_node, tt); //callback?
      //pt._materials.push_back(tt);
    }
    start_node = asset_node;
  } while (asset_node);
}
*/

void PackageBaker::parse_xml(mxml_node_t *tree, PackageTemplate &pt, std::ostream &log)
{
  log << "Parsing package xml..." << endl;
  //read all the materials
  mxml_node_t *asset_node = NULL;
  mxml_node_t *start_node = tree;

  const char *buffer = NULL;

  //parse the working dir
  mxml_node_t *root_dir_node = mxmlFindElement(start_node, tree, "root_dir", NULL, NULL, MXML_DESCEND);
  if (root_dir_node)
  {
    buffer = mxmlGetText(root_dir_node, NULL);
    log << "package root dir: " << buffer << endl;
    pt._root_dir = buffer;
  }

  //parse the path
  mxml_node_t *path_node = mxmlFindElement(start_node, tree, "path", NULL, NULL, MXML_DESCEND);
  if (path_node)
  {
    buffer = mxmlGetText(path_node, NULL);
    log << "path: " << buffer << endl;
    pt._path = buffer;
  }

  //get the output filename
  mxml_node_t *outfile_node = mxmlFindElement(start_node, tree, "output_file", NULL, NULL, MXML_DESCEND);
  if (outfile_node)
  {
    buffer = mxmlGetText(outfile_node, NULL);
    cout << "output package filename: " << buffer << endl;
    pt._output_file = buffer;
  }

  //parse all the shader assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "shader", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      ShaderTemplate st;
      parse_shader_xml(asset_node, st);
      pt._shaders.push_back(st);
    }
    start_node = asset_node;
  } while (asset_node);

  //parse all the material assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "material", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      MaterialTemplate mt;
      parse_material_xml(asset_node, mt);
      pt._materials.push_back(mt);
    }
    start_node = asset_node;
  } while (asset_node);

  //parse all the texture assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "texture", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      TextureTemplate tt;
      parse_texture_xml(asset_node, tt);
      pt._textures.push_back(tt);
    }
    start_node = asset_node;
  } while (asset_node);


  //parse all the mesh assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "mesh", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      MeshTemplate mt;
      parse_mesh_xml(asset_node, mt);
      pt._meshes.push_back(mt);
    }
    start_node = asset_node;
  } while (asset_node);

  //parse all the skeleton assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "skeleton", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      SkeletonTemplate st;
      parse_skeleton_xml(asset_node, st);
      pt._skeletons.push_back(st);
    }
    start_node = asset_node;
  } while (asset_node);

  //parse all the animation assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "animation", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      AnimationTemplate at;
      parse_animation_xml(asset_node, at);
      pt._animations.push_back(at);
    }
    start_node = asset_node;
  } while (asset_node);

  //parse all the ui layout assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "ui_layout", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      UILayoutTemplate ut;
      parse_ui_layout_xml(asset_node, ut);
      pt._ui_layouts.push_back(ut);
    }
    start_node = asset_node;
  } while (asset_node);

  //parse all the lens flares
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "lens_flare", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      BasicTemplate bt;
      parse_basic_xml(asset_node, bt);
      pt._lens_flares.push_back(bt);
    }
    start_node = asset_node;
  } while (asset_node);
}

void PackageBaker::bake(mxml_node_t *tree, std::string output_filename, PackageTemplate &pt, std::ostream &log, std::string tabs)
{
  parse_xml(tree, pt, log);

  _html_logger->set_channel("General");

  const char *mundus_root = std::getenv("MUNDUS_ROOT");
  if (mundus_root)
  {
    log << tabs << "Mundus root: " << mundus_root << endl;
  }
  else
  {
    log << tabs << __CONSOLE_LOG_RED__ << "Environment variable MUNDUS_ROOT not set!!!" << endl;
  }

  if (pt._root_dir.length() > 0)
  {
    //change to said directory
    if (CHDIR(pt._root_dir.c_str()) != 0)
    {
      log << tabs << __CONSOLE_LOG_RED__ << "Could not change directories!" << endl;
    }
  }

  //make sure we succeeded
  char cwd[FILENAME_MAX];
  GETCWD(cwd, sizeof(cwd));
  log << tabs << __CONSOLE_LOG_BLUE__ << "working dir: " << endl << cwd << endl;

  //set up the directory path
  if (pt._path.length() > 0)
  {
    char *tmp_path = new char[(pt._path.length() + 1)];
    strcpy(tmp_path, pt._path.c_str());

    const char delimiter[2] = ";";
    char *token = strtok(tmp_path, delimiter);
    while (token != NULL)
    {
      log << tabs << token << endl;
      asset_path.push_back(std::string(token));
      token = strtok(NULL, delimiter);
    }

    delete tmp_path;
  }

  for (uint32_t i = 0; i < pt._animations.size(); i++)
  {
    _html_logger->set_channel("Animation");
    read_animation_file(pt._animations[i], log);
  }
  for (uint32_t i = 0; i < pt._materials.size(); i++)
  {
    _html_logger->set_channel("Shaders");
    read_material_file(pt._materials[i], log);
  }
  for (uint32_t i = 0; i < pt._meshes.size(); i++)
  {
    _html_logger->set_channel("Geometry");
    read_mesh_file(pt._meshes[i], log);
  }
  for (uint32_t i = 0; i < pt._shaders.size(); i++)
  {
    _html_logger->set_channel("Shaders");
    read_shader_file(pt._shaders[i], tabs, log);
  }
  for (uint32_t i = 0; i < pt._skeletons.size(); i++)
  {
    _html_logger->set_channel("Animation");
    read_skeleton_file(pt._skeletons[i], log);
  }
  for (uint32_t i = 0; i < pt._textures.size(); i++)
  {
    _html_logger->set_channel("General");
    read_texture_file(pt._textures[i], log);
  }
  for (uint32_t i = 0; i < pt._ui_layouts.size(); i++)
  {
    _html_logger->set_channel("General");
    read_ui_layout_file(pt._ui_layouts[i], log);
  }
  for (uint32_t i = 0; i < pt._lens_flares.size(); i++)
  {
    _html_logger->set_channel("VFX");
    read_lens_flare_file(pt._lens_flares[i], log);
  }

  write_package(pt._output_file, log);
}

std::string ShaderPackageAsset::include_shader(std::string inc_fname)
{
  std::string source;
  assert(path);

  for (uint32_t i = 0; i < path->size(); i++)
  {
    std::string full_fname = (*path)[i] + inc_fname;

    FILE *f = fopen(full_fname.c_str(), "r");
    if (f)
    {
      fseek(f, 0, SEEK_END);
      uint32_t fsize = ftell(f);
      rewind(f);

      char *tmp = (char *)malloc(fsize + 1);
      memset(tmp, 0, fsize + 1);
      fread(tmp, fsize, 1, f);
      fclose(f);

      source = tmp;
      free(tmp);
      return source;
    }
  }

  return source;
}

void PackageBaker::parse_basic_xml(mxml_node_t *layout_node, BasicTemplate &bt)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(layout_node, "name");
  bt._name = buffer;
  buffer = mxmlGetText(layout_node, NULL);
  bt._fname = buffer;
}

void ShaderPackageAsset::parse_source(std::string source, std::string *dest)
{
  int last = 0; //must be int for npos comparison to work
  int next = 0; //must be int for npos comparison to work
  std::string line;

  //loop through every line of the source
  while ((next = source.find("\n", last)) != std::string::npos)
  {
    line = source.substr(last, next - last);

    //see if we have a "#include in this line
    int pos = line.find("#include");
    if (pos != std::string::npos)
    {
      uint32_t start_quote = line.find("\"", pos);
      uint32_t end_quote = line.find("\"", start_quote + 1);
      std::string inc_fname = line.substr(start_quote + 1, end_quote - start_quote - 1);
      std::string inc_src = include_shader(inc_fname);
      source.replace(last, next - last, inc_src);
      next = last = 0;
    }
    else
    {
      last = next + 1;
    }
  }

  //copy the new source into our destination
  *dest = source;
}

void PackageBaker::parse_shader_xml(mxml_node_t *shader_node, ShaderTemplate &st)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(shader_node, "name");
  st._name = buffer;

  uint32_t shader_type = 0;
  buffer = mxmlElementGetAttr(shader_node, "type");
  st._type = SHADER_TYPE_RENDER;
  if (buffer && !stricmp(buffer, "compute")) { st._type = SHADER_TYPE_COMPUTE; }

  mxml_node_t *cs_node = mxmlFindElement(shader_node, shader_node, "compute_shader", NULL, NULL, MXML_DESCEND);
  if (cs_node)
  {
    buffer = mxmlGetText(cs_node, NULL);
    st._cs_fname = buffer;
  }

  mxml_node_t *vs_node = mxmlFindElement(shader_node, shader_node, "vertex_shader", NULL, NULL, MXML_DESCEND);
  if (vs_node)
  {
    buffer = mxmlGetText(vs_node, NULL);
    st._vs_fname = buffer;
  }

  mxml_node_t *fs_node = mxmlFindElement(shader_node, shader_node, "fragment_shader", NULL, NULL, MXML_DESCEND);
  if (fs_node)
  {
    buffer = mxmlGetText(fs_node, NULL);
    st._fs_fname = buffer;
  }
}

void PackageBaker::read_shader_file(ShaderTemplate &st, std::string tabs, std::ostream &log)
{
  ShaderPackageAsset *shader_asset = new ShaderPackageAsset(st);
  assets.push_back(shader_asset);

  log << endl << tabs << __CONSOLE_LOG_BLUE__ << "Loading shader \"" << st._name << "\"" << endl;
  tabs = tabs + "\t";

  shader_asset->set_path(&asset_path);

  shader_asset->_type = st._type;

  if (st._cs_fname.length() > 0)
  {
    log << tabs << "opening compute shader file: " << st._cs_fname.c_str() << " ... ";

    FILE *fp = NULL;
    FOPEN(fp, st._cs_fname.c_str(), "r");
    if (fp)
    {
      fseek(fp, 0, SEEK_END);
      int string_size = ftell(fp);
      rewind(fp);

      char *glsl_source = (char *)malloc(sizeof(char) * (string_size + 1));
      memset(glsl_source, 0, string_size + 1);
      fread(glsl_source, sizeof(char), string_size, fp);
      shader_asset->cs_source = glsl_source;
      free(glsl_source);
      fclose(fp);

      log << tabs << __CONSOLE_LOG_GREEN__ << "OK" << endl;
    }
    else
    {
      log << tabs << __CONSOLE_LOG_RED__ << "Could not open file!" << endl;
    }
  }

  if (st._vs_fname.length() > 0)
  {
    log << tabs << "opening vertex shader file: " << st._vs_fname.c_str() << " ... ";

    FILE *fp = NULL;
    FOPEN(fp, st._vs_fname.c_str(), "r");
    if (fp)
    {
      fseek(fp, 0, SEEK_END);
      int string_size = ftell(fp);
      rewind(fp);

      char *glsl_source = (char *)malloc(sizeof(char) * (string_size + 1));
      memset(glsl_source, 0, string_size + 1);
      fread(glsl_source, sizeof(char), string_size, fp);
      shader_asset->vs_source = glsl_source;
      free(glsl_source);
      fclose(fp);

      log << tabs << __CONSOLE_LOG_GREEN__ << "OK" << endl;
    }
    else
    {
      log << tabs << __CONSOLE_LOG_RED__ << "Could not open file!" << endl;
    }
  }

  if(st._fs_fname.length() > 0)
  {
    log << tabs << "opening fragment shader file: " << st._fs_fname.c_str() << " ... ";

    FILE *fp = NULL;
    FOPEN(fp, st._fs_fname.c_str(), "r");
    if (fp)
    {
      fseek(fp, 0, SEEK_END);
      int string_size = ftell(fp);
      rewind(fp);

      char *glsl_source = (char *)malloc(sizeof(char) * (string_size + 1));
      memset(glsl_source, 0, string_size + 1);
      fread(glsl_source, sizeof(char), string_size, fp);
      shader_asset->fs_source = glsl_source;
      free(glsl_source);
      fclose(fp);

      log << tabs << __CONSOLE_LOG_GREEN__ << "OK" << endl;
    }
    else
    {
      log << tabs << __CONSOLE_LOG_RED__ << "ERROR: Could not open file!" << endl;
    }
  }

  shader_asset->parse_source(shader_asset->cs_source, &shader_asset->cs_source);
  shader_asset->parse_source(shader_asset->vs_source, &shader_asset->vs_source);
  shader_asset->parse_source(shader_asset->fs_source, &shader_asset->fs_source);

  //test compile to make sure there are no errors
  Shader s;
  switch (shader_asset->_type)
  {
  case SHADER_TYPE_RENDER:
    s.compile_and_link_from_source(shader_asset->vs_source.c_str(), shader_asset->fs_source.c_str(), tabs, log);
    break;
  case SHADER_TYPE_COMPUTE:
    s.compile_and_link_compute_shader(shader_asset->cs_source.c_str(), tabs, log);
    break;
  }
}

void PackageBaker::parse_material_xml(mxml_node_t *mat_node, MaterialTemplate &mt)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(mat_node, "name");
  mt._name = buffer;
  buffer = mxmlGetText(mat_node, NULL);
  mt._fname = buffer;
}

void PackageBaker::read_material_file(MaterialTemplate &mt, std::ostream &log)
{
  MaterialPackageAsset *mat_asset = new MaterialPackageAsset(mt);
  assets.push_back(mat_asset);

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading material \"" << mt._name << "\"" << endl;
  log << "\tsource file: " << mt._fname << " ... " << endl;

  std::string output_fname = mat_asset->fname + ".bin";
  FILE *fp = fopen(mat_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    MaterialBaker mb;
    mb.bake(tree, output_fname, log);
  }

  //and now open the binary file, read it and add it to the asset
  fp = fopen(output_fname.c_str(), "rb");
  if (fp)
  {
    fseek(fp, 0, SEEK_END);
    long int file_size = ftell(fp);
    rewind(fp);

    //allocate a byte array of the appropriate size
    mat_asset->_file_size = file_size;
    mat_asset->_file_data = new uint8_t[file_size];// (unsigned char *)malloc(file_size);
    fread(mat_asset->_file_data, file_size, 1, fp);
    fclose(fp);
  }
}

void PackageBaker::parse_texture_xml(mxml_node_t *texture_node, TextureTemplate &tt)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(texture_node, "name");
  tt._name = buffer;

  tt._slices = 1;
  buffer = mxmlElementGetAttr(texture_node, "slices");
  if (buffer)
  {
    tt._slices = atoi(buffer);
  }

  buffer = mxmlElementGetAttr(texture_node, "format");
  if (buffer)
  {
    tt._format = buffer;
  }
  buffer = mxmlElementGetAttr(texture_node, "wrap_u");
  if (buffer)
  {
    tt._wrap_u = buffer;
  }
  buffer = mxmlElementGetAttr(texture_node, "wrap_v");
  if (buffer)
  {
    tt._wrap_v = buffer;
  }
  buffer = mxmlElementGetAttr(texture_node, "wrap_w");
  if (buffer)
  {
    tt._wrap_w = buffer;
  }

  buffer = mxmlGetText(texture_node, NULL);
  tt._fname = buffer;
}

void PackageBaker::read_texture_file(TextureTemplate &tt, std::ostream &log)
{
  TexturePackageAsset *texture_asset = new TexturePackageAsset(tt);
  assets.push_back(texture_asset);

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading texture \""  << tt._name << "\"" << endl;
  log << "\ttexture format: " << tt._format << endl;
  log << "\tsource file: " << tt._fname.c_str() << " ... ";

  texture_asset->name = tt._name;
  texture_asset->fname = tt._fname;
  //texture_asset->format = tt._format; //TODO

  SDL_Surface *image = IMG_Load(texture_asset->fname.c_str());
  if (!image)
  {
    log << __CONSOLE_LOG_RED__ << "PackageBaker::read_texture_file() - " << IMG_GetError() << endl;
    return;
  }

  log << __CONSOLE_LOG_GREEN__ << "OK" << endl;

  log << "\twidth: " << image->w / tt._slices << endl;
  log << "\theight: " << image->h << endl;
  log << "\tdepth: " << tt._slices << endl;

  texture_asset->bpp = image->format->BytesPerPixel;
  texture_asset->width = image->w / tt._slices;
  texture_asset->height = image->h;
  texture_asset->depth = tt._slices;

  //copy the texture data to the asset object to be written to the package
  texture_asset->tex_data_size = texture_asset->bpp * image->w * image->h;
  texture_asset->tex_data = malloc(texture_asset->tex_data_size);
  memcpy(texture_asset->tex_data, image->pixels, texture_asset->tex_data_size);

  //deallocate the sdl image
  SDL_FreeSurface(image);
}

void PackageBaker::parse_mesh_xml(mxml_node_t *mesh_node, MeshTemplate &mt)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(mesh_node, "name");
  if (buffer)
  {
    mt._name = buffer;
  }

  buffer = mxmlGetText(mesh_node, NULL);
  if (buffer)
  {
    mt._fname = buffer;
  }
}

void PackageBaker::read_mesh_file(MeshTemplate &mt, std::ostream &log)
{
  MeshPackageAsset *mesh_asset = new MeshPackageAsset(mt);
  assets.push_back(mesh_asset);

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading mesh geometry \"" << mt._name << "\"" << endl;
  log << "\topening source file: " << mt._fname << " ... ";

  std::string output_fname = mesh_asset->fname + ".bin";
  FILE *fp = fopen(mesh_asset->fname.c_str(), "r");
  if (fp)
  {
    log << __CONSOLE_LOG_GREEN__ << "OK" << endl;
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    StaticMeshBaker smb;
    smb.bake(tree, output_fname, log);
  }
  else
  {
    log << __CONSOLE_LOG_RED__ << "Could not open file!" << endl;
  }

  //and now open the binary file, read it and add it to the asset
  fp = fopen(output_fname.c_str(), "rb");
  if (fp)
  {
    int version;
    fread(&version, sizeof(int), 1, fp);

    fread(&mesh_asset->num_verts, sizeof(uint32_t), 1, fp);

    StaticMeshVertex *verts = new StaticMeshVertex[mesh_asset->num_verts];
    fread(verts, sizeof(StaticMeshVertex), mesh_asset->num_verts, fp);

    fread(&mesh_asset->num_indices, sizeof(uint32_t), 1, fp);

    uint32_t *indices = new uint32_t[mesh_asset->num_indices];
    fread(indices, sizeof(uint32_t), mesh_asset->num_indices, fp);

    mesh_asset->vertices = verts;
    mesh_asset->indices = indices;

    fclose(fp);
  }
}

void PackageBaker::parse_skeleton_xml(mxml_node_t *skel_node, SkeletonTemplate &st)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(skel_node, "name");
  st._name = buffer;
  buffer = mxmlGetText(skel_node, NULL);
  st._fname = buffer;
}

void PackageBaker::read_skeleton_file(SkeletonTemplate &st, std::ostream &log)
{
  SkeletonPackageAsset *skeleton_asset = new SkeletonPackageAsset(st);
  assets.push_back(skeleton_asset);

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading Skeleton Rig \"";
  log << st._name.c_str() << "\"" << endl;

  log << "\tsource file: " << st._fname << " ... " << endl;

  std::string output_fname = skeleton_asset->fname + ".bin";
  FILE *fp = fopen(skeleton_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    SkeletonBaker sb;
    sb.bake(tree, output_fname, log);
  }

  //and now open the binary file, read it and add it to the asset
  fp = fopen(output_fname.c_str(), "rb");
  if (fp)
  {
    int version;
    fread(&version, sizeof(int), 1, fp);

    uint32_t num_skeletons;
    fread(&num_skeletons, sizeof(uint32_t), 1, fp);
    fread(&skeleton_asset->num_bones, sizeof(uint32_t), 1, fp);
    skeleton_asset->bones = new Animation::Bone[skeleton_asset->num_bones];
    fread(skeleton_asset->bones, sizeof(Animation::Bone), skeleton_asset->num_bones, fp);

    fclose(fp);
  }
}

void PackageBaker::parse_animation_xml(mxml_node_t *anim_node, AnimationTemplate &at)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(anim_node, "name");
  at._name = buffer;

  buffer = mxmlGetText(anim_node, NULL);
  at._fname = buffer;
}

void PackageBaker::read_animation_file(AnimationTemplate &at, std::ostream &log)
{
  AnimationPackageAsset *animation_asset = new AnimationPackageAsset(at);
  assets.push_back(animation_asset);

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading Animation \"" << at._name << "\"" << endl;
  log << "\tsource file: " << at._fname << " ... " << endl;

  std::string output_fname = at._fname + ".bin";
  FILE *fp = fopen(animation_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    AnimationBaker ab;
    ab.bake(tree, output_fname, log);
  }

  //and now open the binary file, read it and add it to the asset
  fp = fopen(output_fname.c_str(), "rb");
  if (fp)
  {
    //animation_asset->anim.read_from_file(fp);
    int version;
    fread(&version, sizeof(int), 1, fp);

    uint32_t num_tracks;
    fread(&num_tracks, sizeof(uint32_t), 1, fp);

    //std::vector<BoneAnimTrack> tracks;
    for (uint32_t i = 0; i < num_tracks; i++)
    {
      BoneAnimTrack bat;
      fread(&bat._bone_id, sizeof(uint32_t), 1, fp);

      uint32_t num_pos_frames, num_rot_frames, num_scale_frames;
      fread(&num_pos_frames, sizeof(uint32_t), 1, fp);
      fread(&num_rot_frames, sizeof(uint32_t), 1, fp);
      fread(&num_scale_frames, sizeof(uint32_t), 1, fp);

      bat._pos_frames.resize(num_pos_frames);
      bat._rot_frames.resize(num_rot_frames);
      bat._scale_frames.resize(num_scale_frames);

      fread(bat._pos_frames.data(), sizeof(BoneTransformPos), num_pos_frames, fp);
      fread(bat._rot_frames.data(), sizeof(BoneTransformRot), num_rot_frames, fp);
      fread(bat._scale_frames.data(), sizeof(BoneTransformScale), num_scale_frames, fp);

      animation_asset->anim._tracks.push_back(bat);
    }
    //animation_asset->anim.alloc_tracks(tracks.size(), tracks.data());
    fclose(fp);
  }
}

void PackageBaker::parse_ui_layout_xml(mxml_node_t *layout_node, UILayoutTemplate &ut)
{
  const char *buffer = NULL;
  buffer = mxmlElementGetAttr(layout_node, "name");
  ut._name = buffer;
  buffer = mxmlGetText(layout_node, NULL);
  ut._fname = buffer;
}

void PackageBaker::read_ui_layout_file(UILayoutTemplate &ut, std::ostream &log)
{
  UILayoutPackageAsset *layout_asset = new UILayoutPackageAsset(ut);
  layout_asset->set_type(PACKAGE_ASSET_UI_LAYOUT);
  assets.push_back(layout_asset);

  layout_asset->name = ut._name;
  layout_asset->fname = ut._fname;

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading UI Layout \"" << ut._name << "\"" << endl;
  log << "\tsource file: " << ut._fname << " ... ";

  FILE *fp = NULL;
  FOPEN(fp, layout_asset->fname.c_str(), "rt");
  if (fp)
  {
    fseek(fp, 0, SEEK_END);
    int string_size = ftell(fp);
    rewind(fp);

    char *xml_source = (char *)malloc(sizeof(char) * (string_size + 1));
    memset(xml_source, 0, string_size + 1);
    fread(xml_source, sizeof(char), string_size, fp);
    layout_asset->xml_source = xml_source;
    free(xml_source);
    fclose(fp);

    log << __CONSOLE_LOG_GREEN__ << "OK" << endl;
  }
  else
  {
    log << __CONSOLE_LOG_RED__ << "Could not open file!" << endl;
  }
}

void PackageBaker::read_lens_flare_file(BasicTemplate &bt, std::ostream &log)
{
  LensFlarePackageAsset *lens_flare_asset = new LensFlarePackageAsset(bt);
  assets.push_back(lens_flare_asset);

  log << endl << __CONSOLE_LOG_BLUE__ << "Loading Lens Flare \"" << bt._name.c_str() << "\"" << endl;
  log << "\tsource file: " << bt._fname << " ... " << endl;

  std::string output_fname = lens_flare_asset->fname + ".bin";
  FILE *fp = fopen(lens_flare_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);
    fclose(fp);

    LensFlareBaker lfb;
    lfb.bake(tree, output_fname, log);
  }

  //and now open the binary file, read it and add it to the asset
  fp = fopen(output_fname.c_str(), "rb");
  if (fp)
  {
    fseek(fp, 0, SEEK_END);
    long int file_size = ftell(fp);
    rewind(fp);

    //allocate a byte array of the appropriate size
    lens_flare_asset->_file_size = file_size;
    lens_flare_asset->_file_data = new uint8_t[file_size];
    fread(lens_flare_asset->_file_data, file_size, 1, fp);
    fclose(fp);
  }
}

void PackageBaker::write_package(std::string output_filename, std::ostream &log, std::string tabs)
{
  log << endl << tabs.c_str() <<"Writing game package to " << output_filename.c_str() << endl;
  FILE *fp;
  FOPEN(fp, output_filename.c_str(), "wb");
  if (fp)
  {
    //collect and count each asset type
    std::vector<ShaderPackageAsset *>      shaders;
    std::vector<TexturePackageAsset *>     textures;
    std::vector<MaterialPackageAsset *>    materials;
    std::vector<MeshPackageAsset *>        meshes;
    std::vector<SkeletonPackageAsset *>    skeletons;
    std::vector<AnimationPackageAsset *>   animations;
    std::vector<UILayoutPackageAsset *>    ui_layouts;
    std::vector<LensFlarePackageAsset *>   lens_flares;

    for (uint32_t i = 0; i < assets.size(); i++)
    {
      switch (assets[i]->get_type())
      {
      case PACKAGE_ASSET_SHADER:
        shaders.push_back((ShaderPackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_TEXTURE:
        textures.push_back((TexturePackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_MATERIAL:
        materials.push_back((MaterialPackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_MESH:
        meshes.push_back((MeshPackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_SKELETON:
        skeletons.push_back((SkeletonPackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_ANIMATION:
        animations.push_back((AnimationPackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_UI_LAYOUT:
        ui_layouts.push_back((UILayoutPackageAsset *)assets[i]);
        break;
      case PACKAGE_ASSET_LENS_FLARE:
        lens_flares.push_back((LensFlarePackageAsset *)assets[i]);
        break;
      }
    }
    uint32_t shader_count =       shaders.size();
    uint32_t texture_count =      textures.size();
    uint32_t material_count =     materials.size();
    uint32_t mesh_count =         meshes.size();
    uint32_t skeleton_count =     skeletons.size();
    uint32_t animation_count =    animations.size();
    uint32_t ui_layout_count =    ui_layouts.size();
    uint32_t lens_flare_count =   lens_flares.size();

    log << tabs << "Packaging " << shader_count <<       " shaders..." << endl;
    log << tabs << "Packaging " << texture_count <<      " textures..." << endl;
    log << tabs << "Packaging " << material_count <<     " materials..." << endl;
    log << tabs << "Packaging " << mesh_count <<         " meshes..." << endl;
    log << tabs << "Packaging " << skeleton_count <<     " skeletons..." << endl;
    log << tabs << "Packaging " << animation_count <<    " animations..." << endl;
    log << tabs << "Packaging " << ui_layout_count <<    " ui layouts..." << endl;
    log << tabs << "Packaging " << lens_flare_count <<   " lens flares ..." << endl;

    //file header
    fwrite(&file_version, sizeof(uint32_t), 1, fp);
    fwrite(&shader_count, sizeof(uint32_t), 1, fp);
    fwrite(&texture_count, sizeof(uint32_t), 1, fp);
    fwrite(&material_count, sizeof(uint32_t), 1, fp);
    fwrite(&mesh_count, sizeof(uint32_t), 1, fp);
    fwrite(&skeleton_count, sizeof(uint32_t), 1, fp);
    fwrite(&animation_count, sizeof(uint32_t), 1, fp);
    fwrite(&ui_layout_count, sizeof(uint32_t), 1, fp);
    fwrite(&lens_flare_count, sizeof(uint32_t), 1, fp);

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing shader packlets..." << endl;
    for (uint32_t i = 0; i < shaders.size(); i++)
    {
      ShaderPackageAsset *s = shaders[i];
      write_shader_packlet(fp, s, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing texture packlets..." << endl;
    for (uint32_t i = 0; i < textures.size(); i++)
    {
      TexturePackageAsset *t = textures[i];
      write_texture_packlet(fp, t, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing material packlets..." << endl;
    for (uint32_t i = 0; i < materials.size(); i++)
    {
      MaterialPackageAsset *m = materials[i];
      write_material_packlet(fp, m, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing mesh packlets..." << endl;
    for (uint32_t i = 0; i < meshes.size(); i++)
    {
      MeshPackageAsset *m = meshes[i];
      write_mesh_packlet(fp, m, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing skeleton packlets..." << endl;
    for (uint32_t i = 0; i < skeletons.size(); i++)
    {
      SkeletonPackageAsset *m = skeletons[i];
      write_skeleton_packlet(fp, m, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing animation packlets..." << endl;
    for (uint32_t i = 0; i < animations.size(); i++)
    {
      AnimationPackageAsset *m = animations[i];
      write_animation_packlet(fp, m, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing ui layout packlets..." << endl;
    for (uint32_t i = 0; i < ui_layouts.size(); i++)
    {
      UILayoutPackageAsset *u = ui_layouts[i];
      write_ui_layout_packlet(fp, u, tabs, log);
    }

    log << tabs << __CONSOLE_LOG_BLUE__ << "Writing lens flare packlets..." << endl;
    for (uint32_t i = 0; i < lens_flares.size(); i++)
    {
      LensFlarePackageAsset *lf = lens_flares[i];
      write_lens_flare_packlet(fp, lf, tabs, log);
    }

    fclose(fp);
  }
  else
  {
    log << tabs << __CONSOLE_LOG_RED__ << "Could not open file for writing!!!" << endl;
  }
}

void PackageBaker::write_packlet_header(FILE *fp, PackageAsset *a, std::ostream &log)
{
  uint32_t hash_id = Math::hash_value_from_string(a->get_name().c_str());
  log << "\"" << a->get_name() << "\"" << " -> 0x" << std::hex << hash_id << " (" << std::dec << hash_id << ")" << endl;

  uint32_t name_length = (a->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (a->fname.size() + 1) * sizeof(char);

  fwrite(&hash_id, sizeof(uint32_t), 1, fp);
  fwrite(&name_length, sizeof(uint32_t), 1, fp);
  fwrite(&fname_length, sizeof(uint32_t), 1, fp);
}

void PackageBaker::write_shader_packlet(FILE *fp, ShaderPackageAsset *s, std::string tabs, std::ostream &log)
{
  //convert name to hash id
  uint32_t hash_id = Math::hash_value_from_string(s->get_name().c_str());
  log << "\"" << s->get_name() << "\"" << " -> 0x" << std::hex << hash_id << " (" << std::dec << hash_id << ")" << endl;

  uint32_t name_length = (s->name.size() + 1) * sizeof(char);

  uint32_t cs_fname_length = (s->cs_fname.size() + 1) * sizeof(char);
  uint32_t vs_fname_length = (s->vs_fname.size() + 1) * sizeof(char);
  uint32_t fs_fname_length = (s->fs_fname.size() + 1) * sizeof(char);

  uint32_t cs_length = (s->cs_source.size() + 1) * sizeof(char);
  uint32_t vs_length = (s->vs_source.size() + 1) * sizeof(char);
  uint32_t fs_length = (s->fs_source.size() + 1) * sizeof(char);

  uint32_t shader_type = (uint32_t)s->_type;

  fwrite(&hash_id, sizeof(uint32_t), 1, fp);
  fwrite(&name_length, sizeof(uint32_t), 1, fp);
  fwrite(&shader_type, sizeof(uint32_t), 1, fp);

  fwrite(&cs_fname_length, sizeof(uint32_t), 1, fp);
  fwrite(&vs_fname_length, sizeof(uint32_t), 1, fp);
  fwrite(&fs_fname_length, sizeof(uint32_t), 1, fp);
  
  fwrite(&cs_length, sizeof(uint32_t), 1, fp);
  fwrite(&vs_length, sizeof(uint32_t), 1, fp);
  fwrite(&fs_length, sizeof(uint32_t), 1, fp);

  fwrite(s->name.c_str(), sizeof(char), name_length, fp);
  if (shader_type == Graphics::SHADER_TYPE_COMPUTE)
  {
    fwrite(s->cs_fname.c_str(), sizeof(char), cs_fname_length, fp);
    fwrite(s->cs_source.c_str(), sizeof(char), cs_length, fp);
  }
  if (shader_type == Graphics::SHADER_TYPE_RENDER)
  {
    fwrite(s->vs_fname.c_str(), sizeof(char), vs_fname_length, fp);
    fwrite(s->fs_fname.c_str(), sizeof(char), fs_fname_length, fp);
    fwrite(s->vs_source.c_str(), sizeof(char), vs_length, fp);
    fwrite(s->fs_source.c_str(), sizeof(char), fs_length, fp);
  }
}

void PackageBaker::write_texture_packlet(FILE *fp, TexturePackageAsset *t, std::string tabs, std::ostream &log)
{
  write_packlet_header(fp, t, log);

  uint32_t name_length = (t->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (t->fname.size() + 1) * sizeof(char);

  fwrite(&t->bpp, sizeof(uint32_t), 1, fp);
  fwrite(&t->width, sizeof(uint32_t), 1, fp);
  fwrite(&t->height, sizeof(uint32_t), 1, fp);
  fwrite(&t->depth, sizeof(uint32_t), 1, fp);
  fwrite(&t->wrap_u, sizeof(uint32_t), 1, fp);
  fwrite(&t->wrap_v, sizeof(uint32_t), 1, fp);
  fwrite(&t->wrap_w, sizeof(uint32_t), 1, fp);
  fwrite(&(t->tex_data_size), sizeof(uint32_t), 1, fp);

  fwrite(t->name.c_str(), sizeof(char), name_length, fp);
  fwrite(t->fname.c_str(), sizeof(char), fname_length, fp);

  fwrite(t->tex_data, t->tex_data_size, 1, fp);
}

void PackageBaker::write_material_packlet(FILE *fp, MaterialPackageAsset *m, std::string tabs, std::ostream &log)
{
  write_packlet_header(fp, m, log);

  uint32_t name_length = (m->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (m->fname.size() + 1) * sizeof(char);
  fwrite(&m->_file_size, sizeof(uint32_t), 1, fp);

  fwrite(m->name.c_str(), sizeof(char), name_length, fp);
  fwrite(m->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(m->_file_data, sizeof(uint8_t), m->_file_size, fp);
}

void PackageBaker::write_mesh_packlet(FILE *fp, MeshPackageAsset *m, std::string tabs, std::ostream &log)
{
  write_packlet_header(fp, m, log);
  fwrite(&m->num_verts, sizeof(uint32_t), 1, fp);
  fwrite(&m->num_indices, sizeof(uint32_t), 1, fp);

  uint32_t name_length = (m->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (m->fname.size() + 1) * sizeof(char);
  fwrite(m->name.c_str(), sizeof(char), name_length, fp);
  fwrite(m->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(m->vertices, sizeof(StaticMeshVertex), m->num_verts, fp);
  fwrite(m->indices, sizeof(uint32_t), m->num_indices, fp);
}

void PackageBaker::write_skeleton_packlet(FILE *fp, SkeletonPackageAsset *s, std::string tabs, std::ostream &log)
{
  //header
  write_packlet_header(fp, s, log);
  fwrite(&s->num_bones, sizeof(uint32_t), 1, fp);

  uint32_t name_length = (s->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (s->fname.size() + 1) * sizeof(char);

  //data
  fwrite(s->name.c_str(), sizeof(char), name_length, fp);
  fwrite(s->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(s->bones, sizeof(Animation::Bone), s->num_bones, fp);
}

void PackageBaker::write_animation_packlet(FILE *fp, AnimationPackageAsset *a, std::string tabs, std::ostream &log)
{
  //header
  write_packlet_header(fp, a, log);

  uint32_t num_tracks = a->anim.get_num_tracks();
  fwrite(&num_tracks, sizeof(uint32_t), 1, fp);

  log << tabs.c_str() << "track count: " << num_tracks << endl;

  uint32_t name_length = (a->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (a->fname.size() + 1) * sizeof(char);

  //data
  fwrite(a->name.c_str(), sizeof(char), name_length, fp);
  fwrite(a->fname.c_str(), sizeof(char), fname_length, fp);

  for (uint32_t i = 0; i < num_tracks; i++)
  {
    fwrite(&a->anim._tracks[i]._bone_id, sizeof(uint32_t), 1, fp);

    uint32_t num_pos_frames = a->anim._tracks[i]._pos_frames.size();
    uint32_t num_rot_frames = a->anim._tracks[i]._rot_frames.size();
    uint32_t num_scale_frames = a->anim._tracks[i]._scale_frames.size();

    fwrite(&num_pos_frames,    sizeof(uint32_t), 1, fp);
    fwrite(&num_rot_frames,    sizeof(uint32_t), 1, fp);
    fwrite(&num_scale_frames,  sizeof(uint32_t), 1, fp);

    fwrite(a->anim._tracks[i]._pos_frames.data(),    sizeof(BoneTransformPos),    num_pos_frames, fp);
    fwrite(a->anim._tracks[i]._rot_frames.data(),    sizeof(BoneTransformRot),    num_rot_frames, fp);
    fwrite(a->anim._tracks[i]._scale_frames.data(),  sizeof(BoneTransformScale),  num_scale_frames, fp);
  }
}

void PackageBaker::write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u, std::string tabs, std::ostream &log)
{
  write_packlet_header(fp, u, log);

  uint32_t name_length =   (u->name.size() + 1) * sizeof(char);
  uint32_t fname_length =  (u->fname.size() + 1) * sizeof(char);
  uint32_t xml_length =    (u->xml_source.size() + 1) * sizeof(char);

  fwrite(&xml_length, sizeof(uint32_t), 1, fp);

  fwrite(u->name.c_str(), sizeof(char), name_length, fp);
  fwrite(u->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(u->xml_source.c_str(), sizeof(char), xml_length, fp);
}

void PackageBaker::write_lens_flare_packlet(FILE *fp, LensFlarePackageAsset *lf, std::string tabs, std::ostream &log)
{
  write_packlet_header(fp, lf, log);

  //same code as material ... consolidate / genericize?
  uint32_t name_length = (lf->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (lf->fname.size() + 1) * sizeof(char);
  fwrite(&lf->_file_size, sizeof(uint32_t), 1, fp);

  fwrite(lf->name.c_str(), sizeof(char), name_length, fp);
  fwrite(lf->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(lf->_file_data, sizeof(uint8_t), lf->_file_size, fp);
}
