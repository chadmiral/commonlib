#include <iostream>
#include <assert.h>
#include <time.h>

#include "math_utility.h"
#include "matrix.h"
#include "perlin.h"
#include "quaternion.h"
#include "voronoi_sphere.h"
#include "sdl_game.h"
#include "curve.h"
#include "gpu_voronoi.h"

using namespace std;
using namespace Math;
using namespace PerlinNoise;

enum TestMode
{
  TEST_MODE_VORONOI_2D,
  TEST_MODE_VORONOI_3D,
  TEST_MODE_FUNCTIONS,
  TEST_MODE_GPU_VORONOI,
  TEST_MODE_GPU_VORONOI_TEX,
  NUM_TEST_MODES
};

class TestApp : public SDLGame
{
public:
  TestApp() : SDLGame(512, 512, "Math Test")
  {
    Num_starting_points = 1;
    rot_angle = 0.0f;
    function_theta = 0.0f;
    mode = TEST_MODE_GPU_VORONOI;
  }

  ~TestApp() {}

  void set_num_verts(int nv) { Num_starting_points = nv; }
private:

  void render_gl()
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    switch(mode)
    {
      case TEST_MODE_VORONOI_3D:
        render_voronoi_3d();
        break;
      case TEST_MODE_VORONOI_2D:
        render_voronoi_2d();
        break;
      case TEST_MODE_FUNCTIONS:
        render_functions();
        break;
      case TEST_MODE_GPU_VORONOI:
        render_gpu_voronoi();
        break;
      case TEST_MODE_GPU_VORONOI_TEX:
        gpu_voronoi.render_voronoi_texture();
        //render_gpu_voronoi_tex();
        break;
      default:
        break;
    }
  }

  void render_gpu_voronoi()
  {
    Uint32 ticks = SDL_GetTicks();
    float game_time = (float)ticks;

    gpu_voronoi.build_voronoi_diagram();
  }

  void render_fullscreen_quad()
  {
    glBegin(GL_QUADS);
      glColor3f(1.0f, 1.0f, 1.0f);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();
  }

  void setup_textured_quad_state()
  {
    glUseProgramObjectARB(0);
    glDisable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    //glClientActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void render_gpu_voronoi_tex()
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_voronoi.get_tex());
    render_fullscreen_quad();

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
  }

  void render_voronoi_3d()
  {
    Uint32 ticks = SDL_GetTicks();
    float game_time = (float)ticks;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(0.7f, 0.7f, 0.7f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);

    glRotatef(rot_angle, 0.0f, 1.0f, 0.0f);
    rot_angle += 0.3f;

    //render stuff here
    glColor3f(0.7f, 0.1f, 0.0f);
    glBegin(GL_LINES);

      TriangulationSphere *tri = point_cloud.get_triangulation();

      std::vector<Edge3D> *edges = tri->get_edges();
      for(unsigned int i = 0; i < edges->size(); i++)
      {
        Edge3D e = (*edges)[i];
        Float3 a = point_cloud.get_point(e.vidx[0]);
        Float3 b = point_cloud.get_point(e.vidx[1]);

        float pct = (float)i / (float)edges->size();
        Float3 red(1.0f, 0.0f, 0.0f);
        Float3 blue(0.0f, 0.0f, 1.0f);
        Float3 col = lerp(red, blue, pct);
        glColor3f(col[0], col[1], col[2]);
        glVertex3f(a[0], a[1], a[2]);
        glVertex3f(b[0], b[1], b[2]);
      }

    glEnd();

    glPointSize(6.0f);
    //glColor3f(1.0f, 1.0f, 1.0f);
    float bh = point_cloud.get_triangulation()->get_beach_line_height();

    glBegin(GL_POINTS);
      for(uint32_t i = 0; i < point_cloud.get_num_points(); i++)
      {
        Float3 p = point_cloud.get_point(i);
        if(p[1] < bh) { glColor3f(1.0f, 0.0f, 0.0f); }
        else { glColor3f(1.0f, 1.0f, 0.0f); }
        if(fabs(p[1] - bh) < 0.1f && p[1] >= bh)
        {
          glVertex3f(p[0], p[1], p[2]);
        }
      }
    glEnd();

    glPointSize(2.0f);
    glBegin(GL_POINTS);
      for(uint32_t i = 0; i < point_cloud.get_num_points(); i++)
      {
        Float3 p = point_cloud.get_point(i);
        if(p[1] < bh) { glColor3f(1.0f, 0.0f, 0.0f); }
        else { glColor3f(1.0f, 1.0f, 0.0f); }
        if(fabs(p[1] - bh) > 0.1f || p[1] < bh)
        {
          glVertex3f(p[0], p[1], p[2]);
        }
      }
    glEnd();

    //render the beach-line
    int num_segments = 32;
    //float rad = cos(-(M_PI / 2.0f) + M_PI * (bh + 1.0f) / 2.0f);
    float dh = (1.0f - bh);// / 2.0f;
    float rad = sqrt(2.0f * dh - dh * dh);
    glLineWidth(1.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i <= num_segments; i++)
    {
      float theta = 2.0f * (float)M_PI * (float)i / (float)num_segments;
      glVertex3f(cos(theta) * rad, bh, sin(theta) * rad);
    }
    glEnd();

    glPopMatrix();
  }

  void render_voronoi_2d() {}

  void render_functions()
  {
    CurveEndPoint cep[3];
    cep[0].p = Float2(0.0f, 0.0f);

    float r1 = (1.0f + sin(function_theta * 1.0f));
    cep[0].t = Float2(cep[0].p[0] + r1 * cos(function_theta * 1.5f), cep[0].p[1] + r1 * sin(function_theta * 1.5f));


    cep[1].p = Float2(0.5f, 1.0f);

    float r2 = (1.0f + sin(function_theta * 1.0f));
    cep[1].t = Float2(cep[1].p[0] + r2 * cos(function_theta), cep[1].p[1] + r2 * sin(function_theta));

    cep[2].p = Float2(1.0f, 0.0f);
    cep[2].t = Float2(0.9f, 0.0f);

    CurveSegmentBezier csc;
    CurveSegmentLerp csl;
    csc.set_endpoints(cep[0], cep[1]);
    csl.set_endpoints(cep[1], cep[2]);

    Curve c;
    c.add_segment(&csc);
    c.add_segment(&csl);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(3.0f);

    glScalef(1.2f, 1.2f, 0.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_POINTS);
      glVertex3f(cep[0].p[0], cep[0].p[1], 0.0f);
      glVertex3f(cep[1].p[0], cep[1].p[1], 0.0f);
      glVertex3f(cep[2].p[0], cep[2].p[1], 0.0f);
    glEnd();

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
      glVertex3f(cep[0].t[0], cep[0].t[1], 0.0f);
      glVertex3f(cep[1].t[0], cep[1].t[1], 0.0f);
      glVertex3f(cep[2].t[0], cep[2].t[1], 0.0f);
    glEnd();

    glBegin(GL_LINES);
      glVertex3f(cep[0].p[0], cep[0].p[1], 0.0f);
      glVertex3f(cep[0].t[0], cep[0].t[1], 0.0f);

      glVertex3f(cep[1].p[0], cep[1].p[1], 0.0f);
      glVertex3f(cep[1].t[0], cep[1].t[1], 0.0f);

      glVertex3f(cep[2].p[0], cep[2].p[1], 0.0f);
      glVertex3f(cep[2].t[0], cep[2].t[1], 0.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    int num_slices = 100;
    glBegin(GL_LINE_STRIP);
      for(int i = 0; i < num_slices + 1; i++)
      {
        float x = (float)i / (float)num_slices;
        float y = c.evaluate(x);

        glVertex3f(x, y, 0.0f);
      }
    glEnd();

    //draw bounding box
    glBegin(GL_LINE_STRIP);
      glVertex3f(0.0f, 0.0f, 0.0f);
      glVertex3f(1.0f, 0.0f, 0.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glVertex3f(0.0f, 1.0f, 0.0f);
      glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();
  }


  void game_loop(const double game_time, const double frame_time)
  {
    function_theta += (float)frame_time;
    if(function_theta > (float)M_PI * 2.0f) { function_theta -= (float)M_PI * 2.0f; }

    if(mode == TEST_MODE_VORONOI_3D)
    {
      point_cloud.triangulation_step(0.005f);
    }
  }

  void user_init()
  {
    for(int i = 0; i < Num_starting_points; i++)
    {
      Float3 p(random(-1.0f, 1.0f), random(-1.0f, 1.0f), random(-1.0f, 1.0f));
      p.normalize();
      point_cloud.add_point(p);
    }
    point_cloud.triangulate();

    for(int i = 0; i < Num_starting_points; i++)
    {
      gpu_voronoi.add_site(Float2(random(0.0f, 1.0f), random(0.0f, 1.0f)));
    }
    gpu_voronoi.init();
  }
  void user_run() {}
  void user_process_event(const SDL_Event &event)
  {
    switch(event.type)
    {
      case SDL_KEYUP:
        switch(event.key.keysym.sym)
        {
          case ' ':
            {
              gpu_voronoi.reset();
              for(int i = 0; i < Num_starting_points; i++)
              {
                Float2 new_pt(random(0.0f, 1.0f), random(0.0f, 1.0f));
                gpu_voronoi.add_site(new_pt);
              }
              gpu_voronoi.build_voronoi_diagram();
            }
            break;
          case ']':
            mode = (TestMode)((int)mode + 1);
            if(mode >= NUM_TEST_MODES) { mode = TEST_MODE_VORONOI_2D; }
            break;
          case '[':
            mode = (TestMode)((int)mode - 1);
            if(mode < 0) { mode = TEST_MODE_FUNCTIONS; }
            break;
          default:
            break;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if(mode == TEST_MODE_VORONOI_3D)
        {
          Float3 new_vertex;
          cout<<"click: ("<<event.button.x<<", "<<event.button.y<<")"<<endl;
          new_vertex[0] = 2.0f * (((float)event.button.x / (float)resolution[0]) - 0.5f);
          new_vertex[1] = -2.0f * (((float)event.button.y / (float)resolution[1]) - 0.5f);
          new_vertex[2] = random(-1.0f, 1.0f);
          new_vertex.normalize();
          point_cloud.add_point(new_vertex);
          point_cloud.triangulate();
        }
        if(mode == TEST_MODE_GPU_VORONOI || mode == TEST_MODE_GPU_VORONOI_TEX)
        {
          Float2 new_vert;
          new_vert[0] = (float)event.button.x / (float)resolution[0];
          new_vert[1] = 1.0f - (float)event.button.y / (float)resolution[1];
          //gpu_voronoi.add_site(new_vert);
           unsigned int idx = gpu_voronoi.query_nearest_site(new_vert);
           cout<<"idx: "<<idx<<endl;
        }
        break;
    }
  }

  //variables
  VoronoiSphere point_cloud;
  GPUVoronoi2D gpu_voronoi;
  int Num_starting_points;
  float rot_angle;
  TestMode mode;

  float function_theta;
};


int main(int argc, char **argv)
{
  TestApp app;

  srand((unsigned int)time(NULL));

  if(argc > 1)
  {
    app.set_num_verts(atoi(argv[1]));
  }

  float x = remap_range(0.3f, 0.0f, 1.0f, -10.0f, 10.0f);
  cout<<"remap: "<<x<<endl;

  app.init();
  app.run();

  Matrix4x4 mat(3.0f, 7.0f, 2.0f, 5.0f,
                1.0f, 8.0f, 4.0f, 2.0f,
                2.0f, 1.0f, 9.0f, 3.0f,
                5.0f, 4.0f, 7.0f, 1.0f);
  Matrix4x4 inv = mat;
  inv.invert();

  Matrix4x4 ident = mat * inv;

  return 0;
}
