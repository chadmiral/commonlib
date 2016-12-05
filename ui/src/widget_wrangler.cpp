#include "widget_wrangler.h"
#include "gl_error.h"

using namespace UI;
using namespace std;
using namespace Graphics;
using namespace Math;

WidgetWrangler::WidgetWrangler()
{
  focus_idx = 0;
}

void WidgetWrangler::render(const Float3 offset) const
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(viewport[0], viewport[2], viewport[3], viewport[1]);

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glDepthMask(GL_FALSE);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(offset[0], offset[1], offset[2]);

  for(uint32_t i = 0; i < widgets.size(); i++)
  {
    widgets[i]->render();
  }
}

void WidgetWrangler::process_event(const SDL_Event &e, const Float2 offset)
{
  //cout<<"WidgetWrangler::process_event()"<<endl;
  //first, figure out which widget has the focus
  //focus_idx = 0;

  int mouse_x, mouse_y;
  //Uint32 button_state = SDL_GetMouseState(&mouse_x, &mouse_y);
  mouse_x += (int)offset[0];
  mouse_y += (int)offset[1];

  for(uint32_t i = 0; i < widgets.size(); i++)
  {
    Widget *w = widgets[i];
    if (w->is_visible())
    {
      bool mouse_over = w->hit_test(mouse_x, mouse_y);
      if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
      {
        if (mouse_over)
        {
          focus_idx = i;
          cout << "focus_id: " << focus_idx << endl;
          //cout<<"pos: "<<w->get_pos()<<endl;
        }
      }
      w->process_event(e, offset);
    }
  }

  /*
  //next, dispatch event messages
  for(uint32_t i = 0; i < widgets.size(); i++)
  {
    Widget *w = widgets[i];
    //w->process_event(e);
    if(i == focus_idx)
    {
      w->set_focus(true);
    }
    else
    {
      w->set_focus(false);
    }
  }

  Widget *focused = widgets[focus_idx];
  focused->process_event(e, offset);
   */
}

void WidgetWrangler::simulate(const double game_time, const double frame_time)
{
  for(unsigned int i = 0; i < widgets.size(); i++)
  {
    widgets[i]->simulate(game_time, frame_time);
  }
}
