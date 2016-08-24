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
#include "package_baker.h"
#include "math_utility.h"
#include "platform.h"
#include "shader.h"

using namespace Math;
using namespace std;
using namespace Tool;
using namespace Graphics;

void PackageBaker::init()
{
}

void PackageBaker::bake(mxml_node_t *tree, std::string output_filename)
{
  cout << "Parsing package xml..." << endl;

  //read all the materials
  mxml_node_t *asset_node = NULL;
  mxml_node_t *start_node = tree;
  const char *buffer = NULL;

  //parse the working dir
  mxml_node_t *root_dir_node = mxmlFindElement(start_node, tree, "root_dir", NULL, NULL, MXML_DESCEND);
  if (root_dir_node)
  {
    buffer = mxmlGetText(root_dir_node, NULL);
    cout << "package root dir: " << buffer << endl;

    //change to said directory
    if (CHDIR(buffer) != 0)
    {
      SET_TEXT_COLOR(CONSOLE_COLOR_RED);
      cerr << "Could not change directories!" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
  }

  //make sure we succeeded
  char cwd[FILENAME_MAX];
  GETCWD(cwd, sizeof(cwd));
  cout << "working dir: " << cwd << endl;

  //parse the path
  mxml_node_t *path_node = mxmlFindElement(start_node, tree, "path", NULL, NULL, MXML_DESCEND);
  if (path_node)
  {
    buffer = mxmlGetText(path_node, NULL);
    cout << "path: " << buffer << endl;

    char *tmp_path = new char[(strlen(buffer) + 1)];
    strcpy(tmp_path, buffer);

    const char delimiter[2] = ";";
    char *token = strtok(tmp_path, delimiter);
    while (token != NULL)
    {
      cout << token << endl;
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
    cout << "output package filename: " << buffer << endl;
    output_filename = buffer;
  }

  //read all the shader assets
  do
  {
    asset_node = mxmlFindElement(start_node, tree, "shader", NULL, NULL, MXML_DESCEND);
    if (asset_node)
    {
      read_shader_file(asset_node);
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

void ShaderPackageAsset::parse_source(std::string source)
{
  uint32_t last = 0;
  uint32_t next = 0;
  std::string line;

  //loop through every line of the source
  while ((next = source.find("\n", last)) != std::string::npos)
  {
    line = source.substr(last, next - last);

    //see if we have a "#include in this line
    uint32_t pos = line.find("#include");
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

  //cout << source << endl;
}

void PackageBaker::read_shader_file(mxml_node_t *shader_node)
{
  const char *buffer = NULL;
  ShaderPackageAsset *shader_asset = new ShaderPackageAsset;
  shader_asset->set_type(PACKAGE_ASSET_SHADER);
  assets.push_back(shader_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << "\tLoading shader \"";

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
    cout << "\tvs: " << buffer << " ... ";

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
      cerr << "Could not open file!" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
  }

  mxml_node_t *fs_node = mxmlFindElement(shader_node, shader_node, "fragment_shader", NULL, NULL, MXML_DESCEND);
  if (fs_node)
  {
    buffer = mxmlGetText(fs_node, NULL);
    shader_asset->fs_fname = buffer;
    cout << "\tfs: " << buffer << " ... ";

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
      cerr << "Could not open file!" << endl;
      SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    }
  }

  shader_asset->parse_source(shader_asset->vs_source);
  shader_asset->parse_source(shader_asset->fs_source);

  //TODO - test compile to make sure there are no errors
  //Shader s;
  //s.compile_and_link_from_source(shader_asset->vs_source.c_str(), shader_asset->fs_source.c_str());
}

void PackageBaker::read_texture_file(mxml_node_t *texture_node)
{
  const char *buffer = NULL;
  TexturePackageAsset *texture_asset = new TexturePackageAsset;
  texture_asset->set_type(PACKAGE_ASSET_TEXTURE);
  assets.push_back(texture_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << "\tLoading texture \"";
  buffer = mxmlElementGetAttr(texture_node, "name");
  cout << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  texture_asset->set_name(buffer);

  buffer = mxmlElementGetAttr(texture_node, "format");
  cout << "\t\ttexture format: " << buffer << endl;

  buffer = mxmlGetText(texture_node, NULL);
  texture_asset->fname = buffer;
  cout << "\t\tsource file: " << buffer << " ... ";

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

void PackageBaker::read_ui_layout_file(mxml_node_t *layout_node)
{
  const char *buffer = NULL;
  UILayoutPackageAsset *layout_asset = new UILayoutPackageAsset;
  layout_asset->set_type(PACKAGE_ASSET_UI_LAYOUT);
  assets.push_back(layout_asset);

  SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
  cout << "\tLoading texture \"";
  buffer = mxmlElementGetAttr(layout_node, "name");
  cout << buffer << "\"" << endl;
  SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  layout_asset->set_name(buffer);

  buffer = mxmlGetText(layout_node, NULL);
  layout_asset->fname = buffer;
  cout << "\t\tsource file: " << buffer << " ... ";

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
    cerr << "Could not open file!" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
  }
}

void PackageBaker::write_package(std::string output_filename)
{
  cout << endl << "Writing game package to " << output_filename.c_str() << endl;
  FILE *fp;
  FOPEN(fp, output_filename.c_str(), "wb");
  if (fp)
  {
    //collect and count each asset type
    std::vector<ShaderPackageAsset *> shaders;
    std::vector<TexturePackageAsset *> textures;
    std::vector<UILayoutPackageAsset *> ui_layouts;
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
      case PACKAGE_ASSET_UI_LAYOUT:
        ui_layouts.push_back((UILayoutPackageAsset *)assets[i]);
        break;
      }
    }
    uint32_t shader_count = shaders.size();
    uint32_t texture_count = textures.size();
    uint32_t ui_layout_count = ui_layouts.size();

    cout << "Packaging " << shader_count << " shaders..." << endl;
    cout << "Packaging " << texture_count << " textures..." << endl;
    cout << "Packaging " << ui_layout_count << " ui layouts..." << endl;

    //file header
    fwrite(&file_version, sizeof(uint32_t), 1, fp);
    fwrite(&shader_count, sizeof(uint32_t), 1, fp);
    fwrite(&texture_count, sizeof(uint32_t), 1, fp);
    fwrite(&ui_layout_count, sizeof(uint32_t), 1, fp);

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    cout << "Writing shader packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < shaders.size(); i++)
    {
      ShaderPackageAsset *s = shaders[i];
      write_shader_packlet(fp, s);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    cout << "Writing texture packlets..." << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
    for (uint32_t i = 0; i < textures.size(); i++)
    {
      TexturePackageAsset *t = textures[i];
      write_texture_packlet(fp, t);
    }

    SET_TEXT_COLOR(CONSOLE_COLOR_LIGHT_CYAN);
    cout << "Writing ui layout packlets..." << endl;
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
    cerr << "Could not open file for writing!!!" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
  }
}

void PackageBaker::write_shader_packlet(FILE *fp, ShaderPackageAsset *s)
{
  //convert name to hash id
  uint32_t hash_id = Math::hash_value_from_string(s->get_name().c_str());
  cout << "\"" << s->get_name().c_str() << "\"" << " -> " << hash_id << endl;

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

void PackageBaker::write_texture_packlet(FILE *fp, TexturePackageAsset *t)
{
  uint32_t hash_id = Math::hash_value_from_string(t->get_name().c_str());
  cout << "\"" << t->get_name().c_str() << "\"" << " -> " << hash_id << endl;

  uint32_t name_length = (t->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (t->fname.size() + 1) * sizeof(char);

  //cout << "\tbpp: " << t->bpp << endl;
  //cout << "\twidth: " << t->width << endl;
  //cout << "\theight: " << t->height << endl;
  //cout << "\tdata size: " << t->tex_data_size << endl;
  //cout << "\twrap_u: " << t->wrap_u << endl;
  //cout << "\twrap_v: " << t->wrap_v << endl;

  fwrite(&hash_id, sizeof(uint32_t), 1, fp);
  fwrite(&name_length, sizeof(uint32_t), 1, fp);
  fwrite(&fname_length, sizeof(uint32_t), 1, fp);
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

void PackageBaker::write_ui_layout_packlet(FILE *fp, UILayoutPackageAsset *u)
{
  uint32_t hash_id = Math::hash_value_from_string(u->get_name().c_str());
  cout << "\"" << u->get_name().c_str() << "\"" << " -> " << hash_id << endl;

  uint32_t name_length = (u->name.size() + 1) * sizeof(char);
  uint32_t fname_length = (u->fname.size() + 1) * sizeof(char);
  uint32_t xml_length = (u->xml_source.size() + 1) * sizeof(char);

  fwrite(&hash_id, sizeof(uint32_t), 1, fp);
  fwrite(&name_length, sizeof(uint32_t), 1, fp);
  fwrite(&fname_length, sizeof(uint32_t), 1, fp);
  fwrite(&xml_length, sizeof(uint32_t), 1, fp);

  fwrite(u->name.c_str(), sizeof(char), name_length, fp);
  fwrite(u->fname.c_str(), sizeof(char), fname_length, fp);
  fwrite(u->xml_source.c_str(), sizeof(char), xml_length, fp);
}
