#if defined (_WIN32)
#include <Windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <GL/gl.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

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


using namespace Math;
using namespace std;
using namespace Tool;
using namespace Graphics;
using namespace Animation;

void PackageBaker::init()
{
}

void PackageBaker::bake(mxml_node_t *tree, std::string output_filename, std::string tabs)
{
  cout << tabs.c_str() << "Parsing package xml..." << endl;

  //read all the materials
  mxml_node_t *asset_node = NULL;
  mxml_node_t *start_node = tree;
  const char *buffer = NULL;

  //parse the working dir
  mxml_node_t *root_dir_node = mxmlFindElement(start_node, tree, "root_dir", NULL, NULL, MXML_DESCEND);
  if (root_dir_node)
  {
    buffer = mxmlGetText(root_dir_node, NULL);
    cout << tabs.c_str() << "package root dir: " << buffer << endl;

    //change to said directory
    if (CHDIR(buffer) != 0)
    {
      SET_TEXT_COLOR(CONSOLE_COLOR_RED);
      cerr << tabs.c_str() << "Could not change directories!" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
  }

  //make sure we succeeded
  char cwd[FILENAME_MAX];
  GETCWD(cwd, sizeof(cwd));
  cout << tabs.c_str() << "working dir: " << cwd << endl;

  //parse the path
  mxml_node_t *path_node = mxmlFindElement(start_node, tree, "path", NULL, NULL, MXML_DESCEND);
  if (path_node)
  {
    buffer = mxmlGetText(path_node, NULL);
    cout << tabs.c_str() << "path: " << buffer << endl;

    char *tmp_path = new char[(strlen(buffer) + 1)];
    strcpy(tmp_path, buffer);

    const char delimiter[2] = ";";
    char *token = strtok(tmp_path, delimiter);
    while (token != NULL)
    {
      cout << tabs.c_str() << token << endl;
      asset_path.push_back(std::string(token));
      token = strtok(NULL, delimiter);
    }

    delete tmp_path;
  }

  //get the output filename
  mxml_node_t *outfile_node = mxmlFindElement(start_node, tree, "output_file", NULL, NULL, MXML_DESCEND);
  if (outfile_node)
  {
    buffer = mxmlGetText(outfile_node, NULL);
    cout << tabs.c_str() << "output package filename: " << buffer << endl;
    output_filename = buffer;
  }

  //read all the shader assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "shader", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_shader_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the material assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "material", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_material_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the texture assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "texture", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_texture_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the mesh assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "mesh", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_mesh_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the skeleton assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "skeleton", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_skeleton_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the animation assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "animation", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_animation_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the ui layout assets
  start_node = tree;
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "ui_layout", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_ui_layout_file(asset_node);
    }
    start_node = asset_node;
  } while (asset_node);

  //read all the generic data assets
  write_package(output_filename);
}

std::string ShaderPackageAsset::include_shader(std::string inc_fname)
{
  std::string source;
  //cout << "including file " << inc_fname.c_str() << endl;
  assert(path);

  for (uint32_t i = 0; i < path->size(); i++)
  {
    std::string full_fname = (*path)[i] + inc_fname;
    //cout << full_fname << endl;

    FILE *f = fopen(full_fname.c_str(), "r");
    if (f)
    {
      fseek(f, 0, SEEK_END);
      uint32_t fsize = ftell(f);
      rewind(f);

      char *tmp = (char *)malloc(fsize + 1);//new char[(fsize + 1) / sizeof(char)];
      memset(tmp, 0, fsize + 1);
      fread(tmp, fsize, 1, f);
      fclose(f);

      source = tmp;
      free(tmp);//delete tmp;
      return source;
    }
  }

  return source;
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
      //cout << "pasting in this source: " << endl;
      //cout << inc_src.c_str() << endl;
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

void PackageBaker::read_shader_file(mxml_node_t *shader_node, std::string tabs)
{
  tabs = tabs + "\t";
  const char *buffer = NULL;
  ShaderPackageAsset *shader_asset = new ShaderPackageAsset;
  assets.push_back(shader_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << tabs.c_str() << "Loading shader \"";

  buffer = mxmlElementGetAttr(shader_node, "name");
  cout << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  shader_asset->set_name(buffer);
  shader_asset->set_path(&asset_path);

  mxml_node_t *vs_node = mxmlFindElement(shader_node, shader_node, "vertex_shader", NULL, NULL, MXML_DESCEND);
  if (vs_node)
  {
    buffer = mxmlGetText(vs_node, NULL);
    shader_asset->vs_fname = buffer;
    cout << tabs.c_str() << "\tvs: " << buffer << " ... ";

    FILE *fp = NULL;
    FOPEN(fp, buffer, "r");
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
      //TODO: compile / check for errors?

      SET_TEXT_COLOR(CONSOLE_COLOR_GREEN);
      cout << "OK" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
    else
    {
      SET_TEXT_COLOR(CONSOLE_COLOR_RED);
      cerr << tabs.c_str() << "Could not open file!" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
  }

  mxml_node_t *fs_node = mxmlFindElement(shader_node, shader_node, "fragment_shader", NULL, NULL, MXML_DESCEND);
  if (fs_node)
  {
    buffer = mxmlGetText(fs_node, NULL);
    shader_asset->fs_fname = buffer;
    cout << tabs.c_str() << "\tfs: " << buffer << " ... ";

    FILE *fp = NULL;
    FOPEN(fp, buffer, "r");
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

      SET_TEXT_COLOR(CONSOLE_COLOR_GREEN);
      cout << "OK" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
    else
    {
      SET_TEXT_COLOR(CONSOLE_COLOR_RED);
      cerr << tabs.c_str() << "Could not open file!" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
  }

  shader_asset->parse_source(shader_asset->vs_source, &shader_asset->vs_source);
  shader_asset->parse_source(shader_asset->fs_source, &shader_asset->fs_source);

  //TODO - test compile to make sure there are no errors
  Shader s;
  s.compile_and_link_from_source(shader_asset->vs_source.c_str(), shader_asset->fs_source.c_str(), cout);
}

void PackageBaker::read_material_file(mxml_node_t *mat_node, std::string tabs, std::ostream &log)
{
  tabs += "\t";
  const char *buffer = NULL;
  MaterialPackageAsset *mat_asset = new MaterialPackageAsset;
  assets.push_back(mat_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  log << tabs.c_str() << "Loading material \"";
  buffer = mxmlElementGetAttr(mat_node, "name");
  log << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  mat_asset->set_name(buffer);

  buffer = mxmlGetText(mat_node, NULL);
  mat_asset->fname = buffer;
  log << tabs.c_str() << "\tsource file: " << buffer << " ... " << endl;

  std::string output_fname = mat_asset->fname + ".bin";
  FILE *fp = fopen(mat_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    MaterialBaker mb;
    mb.bake(tree, output_fname, tabs);
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

void PackageBaker::read_texture_file(mxml_node_t *texture_node, std::string tabs)
{
  const char *buffer = NULL;
  TexturePackageAsset *texture_asset = new TexturePackageAsset;
  assets.push_back(texture_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << tabs.c_str() << "\tLoading texture \"";
  buffer = mxmlElementGetAttr(texture_node, "name");
  cout << tabs.c_str() << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  texture_asset->set_name(buffer);

  buffer = mxmlElementGetAttr(texture_node, "format");
  cout << tabs.c_str() << "\t\ttexture format: " << buffer << endl;

  buffer = mxmlGetText(texture_node, NULL);
  texture_asset->fname = buffer;
  cout << tabs.c_str() << "\t\tsource file: " << buffer << " ... ";

  SDL_Surface *image = IMG_Load(buffer);
  if (!image)
  {
    SET_TEXT_COLOR(CONSOLE_COLOR_RED);
    cerr << "PackageBaker::read_texture_file() - " << IMG_GetError() << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    return;
  }

  SET_TEXT_COLOR(CONSOLE_COLOR_GREEN);
  cout << "OK" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  cout << "\t\twidth: "<< image->w <<endl;
  cout << "\t\theight: " << image->h << endl;
  texture_asset->bpp = image->format->BytesPerPixel;
  texture_asset->width = image->w;
  texture_asset->height = image->h;

  buffer = mxmlElementGetAttr(texture_node, "wrap_u");
  if (buffer && !stricmp(buffer, "clamp"))
  {
    texture_asset->wrap_u = GL_CLAMP;
  }
  buffer = mxmlElementGetAttr(texture_node, "wrap_v");
  if (buffer && !stricmp(buffer, "clamp"))
  {
    texture_asset->wrap_v = GL_CLAMP;
  }

  //copy the texture data to the asset object to be written to the package
  texture_asset->tex_data_size = texture_asset->bpp * image->w * image->h;
  texture_asset->tex_data = malloc(texture_asset->tex_data_size);
  memcpy(texture_asset->tex_data, image->pixels, texture_asset->tex_data_size);

  //deallocate the sdl image
  SDL_FreeSurface(image);
}

void PackageBaker::read_mesh_file(mxml_node_t *mesh_node, std::string tabs)
{
  const char *buffer = NULL;
  MeshPackageAsset *mesh_asset = new MeshPackageAsset;
  assets.push_back(mesh_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << tabs.c_str() << "\tLoading mesh geometry \"";
  buffer = mxmlElementGetAttr(mesh_node, "name");
  cout << tabs.c_str() << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  mesh_asset->set_name(buffer);

  buffer = mxmlGetText(mesh_node, NULL);
  mesh_asset->fname = buffer;
  cout << tabs.c_str() << "\t\tsource file: " << buffer << " ... " << endl;

  std::string output_fname = mesh_asset->fname + ".bin";
  FILE *fp = fopen(mesh_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    StaticMeshBaker smb;
    smb.bake(tree, output_fname, tabs);
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

void PackageBaker::read_skeleton_file(mxml_node_t *skel_node, std::string tabs)
{
  tabs += "\t";
  const char *buffer = NULL;
  SkeletonPackageAsset *skeleton_asset = new SkeletonPackageAsset;
  assets.push_back(skeleton_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << tabs.c_str() << "Loading Skeleton Rig \"";
  buffer = mxmlElementGetAttr(skel_node, "name");
  cout << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  skeleton_asset->set_name(buffer);

  buffer = mxmlGetText(skel_node, NULL);
  skeleton_asset->fname = buffer;
  cout << tabs.c_str() << "\tsource file: " << buffer << " ... " << endl;

  std::string output_fname = skeleton_asset->fname + ".bin";
  FILE *fp = fopen(skeleton_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    SkeletonBaker sb;
    sb.bake(tree, output_fname, tabs);
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

void PackageBaker::read_animation_file(mxml_node_t *anim_node, std::string tabs)
{
  tabs += "\t";
  const char *buffer = NULL;
  AnimationPackageAsset *animation_asset = new AnimationPackageAsset;
  assets.push_back(animation_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << tabs.c_str() << "Loading Animation \"";
  buffer = mxmlElementGetAttr(anim_node, "name");
  cout << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  animation_asset->set_name(buffer);

  buffer = mxmlGetText(anim_node, NULL);
  animation_asset->fname = buffer;
  cout << tabs.c_str() << "\tsource file: " << buffer << " ... " << endl;

  std::string output_fname = animation_asset->fname + ".bin";
  FILE *fp = fopen(animation_asset->fname.c_str(), "r");
  if (fp)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    assert(tree);

    //don't need the file anymore now that we have the xml tree
    fclose(fp);

    AnimationBaker ab;
    ab.bake(tree, output_fname, tabs);
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

void PackageBaker::read_ui_layout_file(mxml_node_t *layout_node, std::string tabs)
{
  const char *buffer = NULL;
  UILayoutPackageAsset *layout_asset = new UILayoutPackageAsset;
  layout_asset->set_type(PACKAGE_ASSET_UI_LAYOUT);
  assets.push_back(layout_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << tabs.c_str() << "Loading UI Layout \"";
  buffer = mxmlElementGetAttr(layout_node, "name");
  cout << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  layout_asset->set_name(buffer);

  buffer = mxmlGetText(layout_node, NULL);
  layout_asset->fname = buffer;
  cout << tabs.c_str() << "\tsource file: " << buffer << " ... ";

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

    SET_TEXT_COLOR(CONSOLE_COLOR_GREEN);
    cout << "OK" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
  }
  else
  {
    SET_TEXT_COLOR(CONSOLE_COLOR_RED);
    cerr << tabs.c_str() << "Could not open file!" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
  }
}

void PackageBaker::write_package(std::string output_filename, std::string tabs, std::ostream &log)
{
  log << endl << tabs.c_str() <<"Writing game package to " << output_filename.c_str() << endl;
  FILE *fp;
  FOPEN(fp, output_filename.c_str(), "wb");
  if (fp)
  {
    //collect and count each asset type
    std::vector<ShaderPackageAsset *>    shaders;
    std::vector<TexturePackageAsset *>   textures;
    std::vector<MaterialPackageAsset *>  materials;
    std::vector<MeshPackageAsset *>      meshes;
    std::vector<SkeletonPackageAsset *>  skeletons;
    std::vector<AnimationPackageAsset *> animations;
    std::vector<UILayoutPackageAsset *>  ui_layouts;

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
      }
    }
    uint32_t shader_count = shaders.size();
    uint32_t texture_count = textures.size();
    uint32_t material_count = materials.size();
    uint32_t mesh_count = meshes.size();
    uint32_t skeleton_count = skeletons.size();
    uint32_t animation_count = animations.size();
    uint32_t ui_layout_count = ui_layouts.size();

    log << tabs.c_str() << "Packaging " << shader_count << " shaders..." << endl;
    log << tabs.c_str() << "Packaging " << texture_count << " textures..." << endl;
    log << tabs.c_str() << "Packaging " << material_count << " materials..." << endl;
    log << tabs.c_str() << "Packaging " << mesh_count << " meshes..." << endl;
    log << tabs.c_str() << "Packaging " << skeleton_count << " skeletons..." << endl;
    log << tabs.c_str() << "Packaging " << animation_count << " animations..." << endl;
    log << tabs.c_str() << "Packaging " << ui_layout_count << " ui layouts..." << endl;

    //file header
    fwrite(&file_version, sizeof(uint32_t), 1, fp);
    fwrite(&shader_count, sizeof(uint32_t), 1, fp);
    fwrite(&texture_count, sizeof(uint32_t), 1, fp);
    fwrite(&material_count, sizeof(uint32_t), 1, fp);
    fwrite(&mesh_count, sizeof(uint32_t), 1, fp);
    fwrite(&skeleton_count, sizeof(uint32_t), 1, fp);
    fwrite(&animation_count, sizeof(uint32_t), 1, fp);
    fwrite(&ui_layout_count, sizeof(uint32_t), 1, fp);

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing shader packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < shaders.size(); i++)
    {
      ShaderPackageAsset *s = shaders[i];
      write_shader_packlet(fp, s, tabs);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing texture packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < textures.size(); i++)
    {
      TexturePackageAsset *t = textures[i];
      write_texture_packlet(fp, t, tabs);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing material packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < materials.size(); i++)
    {
      MaterialPackageAsset *m = materials[i];
      write_material_packlet(fp, m, tabs);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing mesh packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < meshes.size(); i++)
    {
      MeshPackageAsset *m = meshes[i];
      write_mesh_packlet(fp, m, tabs);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing skeleton packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < skeletons.size(); i++)
    {
      SkeletonPackageAsset *m = skeletons[i];
      write_skeleton_packlet(fp, m, tabs);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing animation packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < animations.size(); i++)
    {
      AnimationPackageAsset *m = animations[i];
      write_animation_packlet(fp, m, tabs);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    log << tabs.c_str() << "Writing ui layout packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < ui_layouts.size(); i++)
    {
      UILayoutPackageAsset *u = ui_layouts[i];
      write_ui_layout_packlet(fp, u);
    }

    fclose(fp);
  }
  else
  {
    SET_TEXT_COLOR(CONSOLE_COLOR_RED);
    log << tabs.c_str() << "Could not open file for writing!!!" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
  }
}

void PackageBaker::write_packlet_header(FILE *fp, PackageAsset *a, std::ostream &log)
{
  uint32_t hash_id = Math::hash_value_from_string(a->get_name().c_str());
  log << "\"" << a->get_name().c_str() << "\"" << " -> " << hash_id << endl;

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
  log << tabs.c_str() << "\"" << s->get_name().c_str() << "\"" << " -> " << hash_id << endl;

  uint32_t name_length = (s->name.size() + 1) * sizeof(char);
  uint32_t vs_fname_length = (s->vs_fname.size() + 1) * sizeof(char);
  uint32_t fs_fname_length = (s->fs_fname.size() + 1) * sizeof(char);

  uint32_t vs_length = (s->vs_source.size() + 1) * sizeof(char);
  uint32_t fs_length = (s->fs_source.size() + 1) * sizeof(char);

  fwrite(&hash_id, sizeof(uint32_t), 1, fp);
  fwrite(&name_length, sizeof(uint32_t), 1, fp);
  fwrite(&vs_fname_length, sizeof(uint32_t), 1, fp);
  fwrite(&fs_fname_length, sizeof(uint32_t), 1, fp);
  fwrite(&vs_length, sizeof(uint32_t), 1, fp);
  fwrite(&fs_length, sizeof(uint32_t), 1, fp);

  fwrite(s->name.c_str(), sizeof(char), name_length, fp);
  fwrite(s->vs_fname.c_str(), sizeof(char), vs_fname_length, fp);
  fwrite(s->fs_fname.c_str(), sizeof(char), fs_fname_length, fp);

  //write the actual source code
  fwrite(s->vs_source.c_str(), sizeof(char), vs_length, fp);
  fwrite(s->fs_source.c_str(), sizeof(char), fs_length, fp);
}

void PackageBaker::write_texture_packlet(FILE *fp, TexturePackageAsset *t, std::string tabs, std::ostream &log)
{
  write_packlet_header(fp, t, log);

  uint32_t name_length = (t->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (t->fname.size() + 1) * sizeof(char);

  fwrite(&t->bpp, sizeof(uint32_t), 1, fp);
  fwrite(&t->width, sizeof(uint32_t), 1, fp);
  fwrite(&t->height, sizeof(uint32_t), 1, fp);
  fwrite(&t->wrap_u, sizeof(uint32_t), 1, fp);
  fwrite(&t->wrap_v, sizeof(uint32_t), 1, fp);
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
  log << tabs.c_str() << "num tracks: " << num_tracks << endl;
  fwrite(&num_tracks, sizeof(uint32_t), 1, fp);

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

    fwrite(&num_pos_frames, sizeof(uint32_t), 1, fp);
    fwrite(&num_rot_frames, sizeof(uint32_t), 1, fp);
    fwrite(&num_scale_frames, sizeof(uint32_t), 1, fp);

    fwrite(a->anim._tracks[i]._pos_frames.data(), sizeof(BoneTransformPos), num_pos_frames, fp);
    fwrite(a->anim._tracks[i]._rot_frames.data(), sizeof(BoneTransformRot), num_rot_frames, fp);
    fwrite(a->anim._tracks[i]._scale_frames.data(), sizeof(BoneTransformScale), num_scale_frames, fp);
  }
}

void PackageBaker::write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u, std::string tabs, std::ostream &log)
{
  //uint32_t hash_id = Math::hash_value_from_string(u->get_name().c_str());
  //log << tabs.c_str() << "\"" << u->get_name().c_str() << "\"" << " -> " << hash_id << endl;

  write_packlet_header(fp, u, log);

  uint32_t name_length = (u->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (u->fname.size() + 1) * sizeof(char);
  uint32_t xml_length = (u->xml_source.size() + 1) * sizeof(char);

  //fwrite(&hash_id, sizeof(uint32_t), 1, fp);
  //fwrite(&name_length, sizeof(uint32_t), 1, fp);
  //fwrite(&fname_length, sizeof(uint32_t), 1, fp);

  fwrite(&xml_length, sizeof(uint32_t), 1, fp);

  fwrite(u->name.c_str(), sizeof(char), name_length, fp);
  fwrite(u->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(u->xml_source.c_str(), sizeof(char), xml_length, fp);
}
