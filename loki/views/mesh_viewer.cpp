#include <Windows.h>
#include "mesh_viewer.h"
#include "texture.h"
#include "matrix.h"

static char vs_src_410[] =
"#version 410 core\n"
"\n"
"uniform mat4 proj_mat;\n"
"\n"
"in vec3 in_xyz;\n"
"in vec2 in_uv0;\n"
"out vec2 uv0;\n"
"\n"
"void main()\n"
"{\n"
"  gl_Position = proj_mat * vec4(in_xyz, 1.0);\n"
"}\n";

static char fs_src_410[] =
"#version 410 core\n"
"\n"
"in vec2 uv0;\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"  color = vec4(1.0, uv0.s, 0.0, 1.0);"
"}\n";


static char mesh_vs_src[] =
"#version 410 core\n"
"\n"
"uniform mat4 proj_mat;\n"
"uniform mat4 modelview_mat;\n"
"\n"
"in vec3 in_xyz;\n"
"\n"
"void main()\n"
"{\n"
"  gl_Position = proj_mat * modelview_mat * vec4(in_xyz, 1.0);\n"
"}\n";

static char mesh_fs_src[] =
"#version 410 core\n"
"\n"
"out vec4 final_color;\n"
"\n"
"void main()\n"
"{\n"
"  final_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n";


using namespace Graphics;
using namespace Math;

MeshViewer::MeshViewer()
{
  _mesh = NULL;
  _mesh_mat = NULL;
  _mesh_shader = NULL;

  visible = true;
}

void MeshViewer::load_mesh()
{
  char szFile[100];
  OPENFILENAME ofn;

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);

  if (_mesh) { delete _mesh; }
  _mesh = new StaticMesh;

  FILE *f;
  FOPEN(f, ofn.lpstrFile, "rb");
  if (f)
  {
    _mesh->read_from_file(f);
  }
  _mesh->init();
}

void MeshViewer::render_mesh()
{
  //glViewport(0.0f, 0.0f, (GLsizei)_view_size.x, (GLsizei)_view_size.y);

  //set up model view transform
  Matrix4x4 model_view, projection;
  model_view.identity();
  _cam.set_window_dimensions(Float2(_view_size.x, _view_size.y));
  _cam.get_projection_matrix(&projection);

  _mesh_projection.set_var(projection);
  _mesh_modelview.set_var(model_view);

  glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (!_mesh) { return; }

  _mesh->render(_mesh_mat);
}

void MeshViewer::init()
{
  _cam.set_pos(Float3(-100.0f, 0.0f, 0.0f));
  _cam.set_lookat(Float3(1.0f, 0.0f, 0.0f));
  _cam.set_up(Float3(0.0f, 1.0f, 0.0f));

  _render_target_shader.compile_and_link_from_source(vs_src_410, fs_src_410);
  _render_target.set_shader(&_render_target_shader);
  _render_target.init();

  _mesh_shader = new Shader;
  _mesh_shader->compile_and_link_from_source(mesh_vs_src, mesh_fs_src);

  _mesh_mat = new Material;
  _mesh_mat->set_shader(_mesh_shader);

  _mesh_xyz_attrib.set_loc(_mesh_shader, "in_xyz", sizeof(StaticMeshVertex), 3, 0 * sizeof(float));
  _mesh_mat->add_vertex_attrib(&_mesh_xyz_attrib);

  _mesh_projection.set_name("proj_mat");
  _mesh_projection.set_loc(_mesh_shader);
  _mesh_mat->add_uniform_var(&_mesh_projection);

  _mesh_modelview.set_name("modelview_mat");
  _mesh_modelview.set_loc(_mesh_shader);
  _mesh_mat->add_uniform_var(&_mesh_modelview);

  _mesh_mat->enable_backface_culling(false);
  _mesh_mat->init();

  _mesh = NULL;
}

void MeshViewer::deinit()
{
  if (_mesh) { delete _mesh; }
  if (_mesh_shader) { delete _mesh_shader; }
  if (_mesh_mat) { delete _mesh_mat; }
}

void MeshViewer::render()
{
  if (!visible) { return; }
  ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiSetCond_FirstUseEver);
  ImGui::Begin("Mesh Tools", &visible, ImGuiWindowFlags_MenuBar);
  ImGui::Text("");

  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Open Mesh...", NULL, false))
      {
        load_mesh();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  //calculate the size of the render view
  _view_size = ImGui::GetWindowSize();
  ImVec2 view_min = ImGui::GetWindowContentRegionMin();
  ImVec2 view_max = ImGui::GetWindowContentRegionMax();
  _view_size.x -= view_min.x + (_view_size.x - view_max.x) + 2;
  _view_size.y -= view_min.y + (_view_size.y - view_max.y) + 20;

  //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
  //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  //ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(60, 60, 70, 200));

  //render the mesh to the FBO
  int32_t fbo_w, fbo_h;
  _render_target.get_fbo_res(fbo_w, fbo_h);
  if (fbo_w != (int32_t)_view_size.x || fbo_h != (int32_t)_view_size.y)
  {
    _render_target.resize((int32_t)_view_size.x, (int32_t)_view_size.y);
  }
  
  _render_target.capture();
    render_mesh();
  _render_target.release();

  Texture2D *fbo_tex = _render_target.get_tex();
  int32_t t = fbo_tex->get_tex_id();
  ImTextureID tex_id = (ImTextureID)fbo_tex->get_tex_id();//ImGui::GetIO().Fonts->TexID;
  ImGui::Image(tex_id, _view_size, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));

  ImGui::End();
}