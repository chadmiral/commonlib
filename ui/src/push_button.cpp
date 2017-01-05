//
// The first push_button class (and the initial UI code) was written while on vacation
// visiting Lila's parents in Colorado. We stayed in a funny trailer, and shot bows and arrows
//
// CF, 2015
//

#include <iostream>
#include "push_button.h"
#include "gl_error.h"

using namespace std;
using namespace UI;
using namespace Graphics;
using namespace Math;

PushButton::PushButton(Font *f) : Label(f)
{
  click_capture = false;
  click_callback = NULL;
  textures[0] = textures[1] = NULL;
  click_rgb = Float3(0.8f, 0.8f, 0.8f);
  widget_type = WIDGET_PUSH_BUTTON;
}

void PushButton::set_click_callback(void (*cb)(const SDL_Event &e))
{
  click_callback = cb;
}

void PushButton::set_texture(const int i, Texture2D *t)
{
  textures[i] = t;
}

void PushButton::process_event(const SDL_Event &event, const Math::Float2 offset)
{
  int mouse_x, mouse_y;
  Uint32 button_state = SDL_GetMouseState(&mouse_x, &mouse_y);
  mouse_x += (int)offset[0];
  mouse_y += (int)offset[1];

  bool mouse_over = hit_test(mouse_x, mouse_y);
  hovering = mouse_over;

  if (!enabled) { return; } //TODO another callback function to handle (play a sound or something)

  if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
  {
    if(mouse_over)
    {
      click_capture = true;
    }
  }
  if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
  {
    if(click_capture && mouse_over)
    {
      //cout<<"PushButton::process_event(): button clicked!"<<endl;
      if(click_callback)
      {
        click_callback(event);
      }
      //cout<<"button clicked!"<<endl;
    }
    click_capture = false;
  }
}

void PushButton::render()
{
  gl_check_error(std::cout);

  glLineWidth(1.0f);
  if(click_capture)
  {
    glColor4f(click_rgb[0], click_rgb[1], click_rgb[2], 1.0f);
  }
  else
  {
    glColor4f(rgb[0], rgb[1], rgb[2], 1.0f);
  }

/*
  glBegin(GL_LINES);
    glVertex3f(pos[0], pos[1], 0.0f);
    glVertex3f(pos[0] + dim[0], pos[1], 0.0f);

    glVertex3f(pos[0], pos[1] - dim[1], 0.0f);
    glVertex3f(pos[0] + dim[0], pos[1] - dim[1], 0.0f);

    glVertex3f(pos[0], pos[1], 0.0f);
    glVertex3f(pos[0], pos[1] - dim[1], 0.0f);

    glVertex3f(pos[0] + dim[0], pos[1], 0.0f);
    glVertex3f(pos[0] + dim[0], pos[1] - dim[1], 0.0f);
  glEnd();*/

  if (enabled)
  {
    if (textures[0] && !click_capture)
    {
      textures[0]->render_gl();
    }
    else if (textures[1] && click_capture)
    {
      textures[1]->render_gl();
    }
  }
  else
  {
    textures[2]->render_gl();
  }
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(pos[0], pos[1], 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(pos[0] + dim[0], pos[1], 0.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(pos[0], pos[1] - dim[1], 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(pos[0] + dim[0], pos[1] - dim[1], 0.0f);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  if(text.size() > 0)
  {
    Label::render();
  }
  if (hovering) { render_tooltip(); }
}

void PushButton::simulate(const double game_time, const double frame_time)
{
  //cout << "PushButton::simulate()" << endl;
}