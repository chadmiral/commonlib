#include <assert.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <GL/glew.h>

#include "sdl_game.h"
#include "label.h"
#include "menu.h"
#include "toolbox.h"

using namespace std;
using namespace Math;
using namespace UI;
using namespace Graphics;
using namespace Game;

void quit()
{
  SDL_Quit();
  exit(0);
}

void quit_cb(const SDL_Event &e)
{
  quit();
}

SDLGame::SDLGame(const int w, const int h,
                 std::string title,
                 const unsigned int _flags,
                 const int _gl_context_profile,
                 const int gl_major_version, const int gl_minor_version) :
  _html_logger("SDLGame Log"),
  _log(&_html_logger)
{

  _html_logger.open();

  flags = _flags;

  game_context.window_resolution[0] = w;
  game_context.window_resolution[1] = h;

  gl_version[0] = gl_major_version;
  gl_version[1] = gl_minor_version;

  gl_context_profile = _gl_context_profile;

  win = NULL;
  gl_context = NULL;

  window_title = title;

  last_game_time = 0.0f;
  game_context.switch_board.assign_socket(ChannelSocketDouble(&last_game_time), "game_time");

  fps_idx = 0;
  for(int i = 0; i < SDL_GAME_NUM_FPS_FRAMES; i++)
  {
    prev_fps[i] = 30.0f;
  }

  recording_movie = false;
  movie_frame_counter = 0;

  font = NULL;
#if defined(__APPLE__)
  font_face = "/Library/Fonts/Andale Mono.ttf";
  game_context.widget_font_face = "/Library/Fonts/Andale Mono.ttf";
#endif //__APPLE__
#if defined(_WIN32)
  font_face = "C:\\Windows\\Fonts\\Arial.ttf";
  game_context.widget_font_face = "C:\\Windows\\Fonts\\Arial.ttf";
#endif
  font_size = 24;
  game_context.widget_font_size = 12;

  pause_menu = NULL;
  if(flags & SDL_GAME_GENERATE_PAUSE_MENU)
  {
    pause_menu = new Menu;
  }

  game_state = 0;

  sim_lock_dt = 1.0f / 30.0f;
  vsync_enabled = true;
  diy_window_swap = false;
}

SDLGame::~SDLGame()
{
  if(gl_context)
  {
    SDL_GL_DeleteContext(gl_context);
    gl_context = NULL;
  }
  if(win)
  {
    SDL_DestroyWindow(win);
    win = NULL;
  }
  if(pause_menu && (flags & SDL_GAME_GENERATE_PAUSE_MENU))
  {
    delete pause_menu;
  }
  _html_logger.close();
}

void SDLGame::set_resolution(const unsigned int w, const unsigned int h)
{
  game_context.window_resolution[0] = w;
  game_context.window_resolution[1] = h;

  if(win)
  {
    SDL_DisplayMode mode;
    SDL_GetWindowDisplayMode(win, &mode);
    mode.w = w;
    mode.h = h;
    SDL_SetWindowDisplayMode(win, &mode);
  }
}

void SDLGame::toggle_fullscreen()
{
  if(win)
  {
    game_state = game_state ^ SDL_GAME_STATE_FULLSCREEN;
    SDL_SetWindowFullscreen(win, (game_state & SDL_GAME_STATE_FULLSCREEN) ? SDL_WINDOW_FULLSCREEN : 0);
  }
}

void SDLGame::init()
{
  //load init file and set resolution
#ifndef _VR
  FILE *init_file = fopen("init.txt", "r");

  if(init_file)
  {
    fscanf(init_file, "%i", &game_context.window_resolution[0]);
    fscanf(init_file, "%i", &game_context.window_resolution[1]);
    fclose(init_file);
  }
#endif //_VR

  init_sdl();
  font = new Font(font_face.c_str(), font_size);
  font->init();
  title_screen.set_font(font);

  game_context.widget_font = new Font(game_context.widget_font_face.c_str(), game_context.widget_font_size);
  game_context.widget_font->init();
  game_context.console.set_font(game_context.widget_font);

  fps_label.set_font(game_context.widget_font);
  fps_label.set_text(std::string("fps"));
  fps_label.init();
  fps_label.translate(Float2(10.0f, 10.0f));
  fps_label.show();

  if(pause_menu)
  {
    pause_menu->translate(Float2(100.0f, 100.0f));

    //pause_menu->add_menu_item(std::string("Options"), quit_cb);
    //pause_menu->add_menu_item(std::string("Save"), quit_cb);
    //pause_menu->add_menu_item(std::string("Quite a long string"), quit_cb);
    pause_menu->add_menu_item(std::string("Quit"), quit_cb);

    pause_menu->set_font(font);
    pause_menu->init();

    //move to the middle of the screen
    Float2 dim = pause_menu->get_dim();
    float x = game_context.window_resolution[0] / 2.0f - dim[0] / 2.0f;
    float y = game_context.window_resolution[1] / 2.0f - dim[1] / 2.0f;
    pause_menu->translate(Float2(x, y));
  }

  user_init();
  game_context.console.init();
  game_context.console.register_function(quit, "quit");
}

void SDLGame::run()
{
  title_screen.play();
  title_screen.set_text(window_title);

  Uint32 ticks = SDL_GetTicks();
  last_game_time = (double)ticks;
  while(true)
  {
    ticks = SDL_GetTicks();
    double game_time = (double)ticks;
    double frame_time = (game_time - last_game_time) / 1000.0f;
    last_game_time = game_time;

    if(flags & SDL_GAME_LOCK_SIM_DT)
    {
      frame_time = sim_lock_dt;
    }

    //average the last n frames
    float actual_fps = (float)(1.0f / frame_time);
    prev_fps[fps_idx] = actual_fps;
    fps_idx = (fps_idx + 1) % SDL_GAME_NUM_FPS_FRAMES;
    float avg_fps = 0.0f;
    for(int i = 0; i < SDL_GAME_NUM_FPS_FRAMES; i++)
    {
      avg_fps += prev_fps[i];
    }
    avg_fps /= (float)SDL_GAME_NUM_FPS_FRAMES;

    if(!title_screen.is_active())
    {
      user_run();
    }
    process_events();
    if(!title_screen.is_active())
    {
      game_loop(game_time, frame_time);
    }

    std::stringstream ss;
    ss<<std::fixed<<std::setprecision(2)<<avg_fps;

    std::string fps_text = std::string("fps: ") + ss.str();
    fps_label.set_text(fps_text);
    fps_label.simulate(game_time, frame_time);

    if(title_screen.is_active())
    {
      title_screen.simulate((float)frame_time);
      title_screen.render_gl();
    }
    else
    {
      render_gl();

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      fps_label.render();

      if(pause_menu)
      {
        pause_menu->simulate(game_time, frame_time);
        pause_menu->render();
      }

      game_context.console.simulate(game_time, frame_time);
      game_context.console.render_gl();
    }

    if(recording_movie)
    {
      screenshot();
    }

    if (!diy_window_swap)
    {
      //glFlush();
      //glFinish(); //needed? (wait for all GL commands to finish) interacts w/ vsync
      SDL_GL_SwapWindow(win);
    }
  }
}

void SDLGame::process_events()
{
  //base level of event processing every app should have
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  assert(keystate);
  if(keystate[SDL_SCANCODE_ESCAPE])
  {
    if(!pause_menu) { quit_app(); }
  }

  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    //this bit here is pretty terrible
    if (pause_menu && pause_menu->is_visible())
    {
      pause_menu->process_event(event);
      if (!pause_menu->is_visible())
      {
        game_state &= ~SDL_GAME_STATE_PAUSED;
      }
      return;
    }

    if (event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONDOWN)
    {
      if (title_screen.is_active())
      {
        title_screen.stop();
        return;
      }
    }

    if (event.type == SDL_KEYUP)
    {
      switch (event.key.keysym.sym)
      {
      case '`':
        if (keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT])
        {
          game_context.console.activate(game_context.console.is_control_board_active() ? CONSOLE_INACTIVE : CONSOLE_ACTIVE_CONTROL_BOARD);
        }
        else
        {
          game_context.console.activate(game_context.console.is_active() ? CONSOLE_INACTIVE : CONSOLE_ACTIVE_DEFAULT);
        }
        break;
      case SDLK_ESCAPE:
        if (pause_menu && !pause_menu->is_visible())
        {
          game_state |= SDL_GAME_STATE_PAUSED;
          pause_menu->show();
        }
        break;
      case SDLK_UP:
        if (game_context.console.is_active()) { game_context.console.traverse_command_history(1); }
        break;
      case SDLK_DOWN:
        if (game_context.console.is_active()) { game_context.console.traverse_command_history(-1); }
        break;
      case SDLK_RETURN:
        //case SDLK_ENTER:
        if (game_context.console.is_active()) { game_context.console.execute(); }
        if (keystate[SDL_SCANCODE_LALT]) { toggle_fullscreen(); }
        break;
      case SDLK_TAB:
        if (game_context.console.is_active()) { game_context.console.tab_complete(); }
        break;
      case SDLK_BACKSPACE:
        if (game_context.console.is_active()) { game_context.console.backspace(); }
        break;
      case SDLK_F12:
        if (keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT])
        {
          recording_movie = !recording_movie;
          if (recording_movie) { begin_video_capture(); }
          else { end_video_capture(); }
        }
        else { screenshot(); }
        break;
      default:
        break;
      }
    }

    if(event.type == SDL_TEXTINPUT)
    {
      if(game_context.console.is_active())
      {
        if(event.text.text[0] != '`')
        {
          game_context.console.receive_char(event.text.text[0]);
        }
      }
    }

    if(event.type == SDL_QUIT) { quit_app(); }
    if(!game_context.console.is_active()) { user_process_event(event); }
    if(game_context.console.is_control_board_active()) { game_context.console.process_event(event); }
  }
}

void SDLGame::generate_ui_from_layout(std::string name)
{
  //assert(asset_library);
  GameAsset *asset = game_context.asset_library.retrieve_asset(name);
  assert(asset->type == UI_LAYOUT_ASSET);
  Layout *ui_layout = ((UILayoutAsset *)asset)->l;
  assert(ui_layout);

  //first load all the root level widgets
  for (uint32_t i = 0; i < ui_layout->templates.size(); i++)
  {
    LayoutWidgetTemplate *w = ui_layout->templates[i];

    if (w->type == WIDGET_TOOLBOX)
    {
      Float2 max_dim;
      for (uint32_t j = 0; j < w->children.size(); j++)
      {
        LayoutWidgetTemplate *c = w->children[j];
        max_dim[0] = max(max_dim[0], c->dim[0]);
        max_dim[1] = max(max_dim[1], c->dim[1]);
      }

      if (w->flags & UI_LAYOUT_FLAG_VERTICAL)
      {
        w->dim[0] = max_dim[0];
        w->dim[1] = (max_dim[1] + TOOLBOX_DEFAULT_MARGIN_PX) * w->children.size();
      }
      else
      {
        w->dim[0] = (max_dim[0] + TOOLBOX_DEFAULT_MARGIN_PX) * w->children.size();
        w->dim[1] = max_dim[1];
      }
    }

    //fix alignments
    if (w->flags & UI_LAYOUT_FLAG_ALIGN_CENTER_X)
    {
      float center_x = (float)game_context.window_resolution[0] / 2.0f;
      w->offset[0] = center_x - (w->dim[0] / 2.0f) + w->offset[0];
    }
    if (w->flags & UI_LAYOUT_FLAG_ALIGN_RIGHT)
    {
      w->offset[0] = (float)game_context.window_resolution[0] - w->offset[0] - w->dim[0];
    }
    if (w->flags & UI_LAYOUT_FLAG_ALIGN_BOTTOM)
    {
      w->offset[1] = (float)game_context.window_resolution[1] - w->offset[1] - w->dim[1];
    }

    if (w->type != WIDGET_TOOLBOX && w->type != WIDGET_RADIO_GROUP)
    {
      RectangularWidget *rw = NULL;
      switch (w->type)
      {
        case WIDGET_CHECK_BUTTON:
        {
          CheckButton *cb = new CheckButton;
          rw = cb;
          cb->set_texture(0, game_context.asset_library.retrieve_texture(w->tex_default));
          cb->set_texture(1, game_context.asset_library.retrieve_texture(w->tex_active));
          cb->set_texture(2, game_context.asset_library.retrieve_texture(w->tex_off));
          cb->set_click_callback(ui_callback_map[hash_value_from_string(w->click_callback.c_str())]);
          break;
        }
        case WIDGET_PUSH_BUTTON:
        {
          PushButton *pb = new PushButton;
          rw = pb;
          pb->set_texture(0, game_context.asset_library.retrieve_texture(w->tex_default));
          pb->set_texture(1, game_context.asset_library.retrieve_texture(w->tex_active));
          pb->set_texture(2, game_context.asset_library.retrieve_texture(w->tex_off));
          pb->set_text("");
          pb->set_click_callback(ui_callback_map[hash_value_from_string(w->click_callback.c_str())]);
          break;
        }
        default:
          assert(false);
      }

      rw->set_font(game_context.widget_font);
      rw->scale(w->dim);
      rw->translate(w->offset);
      rw->set_tooltip(w->tool_tip);
      rw->init();
      w->real_widget = rw;

      if (!w->parent)
      {
        rw->show();
        ww.add_widget(rw);
      }
    }
  }

  for(uint32_t i = 0; i < ui_layout->templates.size(); i++)
  {
    LayoutWidgetTemplate *w = ui_layout->templates[i];
    switch (w->type)
    {
      case WIDGET_TOOLBOX:
      {
        ToolBox *tb = new ToolBox;
        tb->scale(w->dim);
        tb->translate(w->offset);
        for (uint32_t j = 0; j < w->children.size(); j++)
        {
          LayoutWidgetTemplate *child = w->children[j];
          assert(child);
          if (child->real_widget)
          {
            tb->add_button((PushButton *)child->real_widget);
            tb->set_button_dim(child->dim);
          }
        }

        if (w->flags & UI_LAYOUT_FLAG_VERTICAL)
        {
          tb->make_vertical();
        }
        tb->set_font(game_context.widget_font);
        tb->init();
        tb->show();
        w->real_widget = tb;
        ww.add_widget(tb);

        break;
      }
      case WIDGET_RADIO_GROUP:
      {
        RadioGroup *rg = new RadioGroup;
        for (uint32_t j = 0; j < w->children.size(); j++)
        {
          LayoutWidgetTemplate *child = w->children[j];
          assert(child);
          if (child->real_widget)
          {
            assert(child->real_widget->get_widget_type() == WIDGET_CHECK_BUTTON);
            CheckButton *cb = (CheckButton *)child->real_widget;
            rg->add((CheckButton *)cb);
            cb->set_radio_group(rg);
          }
        }
        break;
      }
      default:
        break;
    }
  }

  //ok, we're all done w/ these templates... deallocate
}


void SDLGame::set_main_font(std::string font_face_name, unsigned int size)
{
  font_face = font_face_name;
  font_size = size;
}

void SDLGame::set_widget_font(std::string font_face_name, unsigned int size)
{
  game_context.widget_font_face = font_face_name;
  game_context.widget_font_size = size;
}

void SDLGame::screenshot()
{
  game_context.console.console_log<<"taking screenshot..."<<endl;
  SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, game_context.window_resolution[0], game_context.window_resolution[1], 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);

  glReadPixels(0, 0, game_context.window_resolution[0], game_context.window_resolution[1], GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

  char filename[256];
  sprintf(filename, "capture/%s%i.bmp", window_title.c_str(), movie_frame_counter++);
  game_context.console.console_log<<"writing "<<filename<<endl;
  SDL_SaveBMP(image, filename);
  SDL_FreeSurface(image);
}

void SDLGame::set_sdl_attributes()
{
  //TODO: should probably query to hardware first to determine support
  //or - maybe just offer an option of core vs. compatibility contexts
  if (gl_version[0] > 0 && gl_version[1] > 0)
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_version[0]);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_version[1]);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  }
  else
  {
    // seems like it always wants 4.1 (or probably the largest supported version
    // when I call this function)
    // NOTE: no immediate mode functionality will work with PROFILE_CORE
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, gl_context_profile);
  }

  //from OpenVR setup (flag for vr?)
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
  //if (m_bDebugOpenGL)
  //  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
}

void SDLGame::sdl_init_verbose()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return;
  }
}

void SDLGame::init_sdl_gl_context()
{
  //Create window
  win = SDL_CreateWindow(window_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, game_context.window_resolution[0], game_context.window_resolution[1], SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  game_context.sdl_window = win;
  assert(win);

  gl_context = SDL_GL_CreateContext(win);
  game_context.sdl_gl_context = &gl_context;
  assert(gl_context);

  glewExperimental = GL_TRUE; //openVR does this?
  glewInit();

  int res = SDL_GL_SetSwapInterval(vsync_enabled ? 1 : 0);
  assert(res == 0);

  game_context.console.console_log << "Initializing OpenGL..." << endl;
  game_context.console.console_log << "version " << glGetString(GL_VERSION) << endl;//major_version<<"."<<minor_version<<endl;
                                                                                    //console.log<<"extensions: "<<endl<<glGetString(GL_EXTENSIONS)<<endl;

  int max_vertex_attribs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
  game_context.console.console_log << "Max Vertex Attribs: " << max_vertex_attribs << endl;
}

//to allow easy overriding
void SDLGame::init_sdl_default()
{
  sdl_init_verbose();
  set_sdl_attributes();
  init_sdl_gl_context();
  game_controller_context.init();
}

void SDLGame::init_sdl()
{
  init_sdl_default();
}

void SDLGame::quit_app()
{
    game_controller_context.deinit();

    SDL_GL_DeleteContext(gl_context);
    gl_context = NULL;

    SDL_DestroyWindow(win);
    win = NULL;

    SDL_Quit();
    exit(0);
}
