#ifndef __SDL_GAME_H__
#define __SDL_GAME_H__

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#if defined(_WIN32)
#include <Windows.h>
#include <GL/glew.h>
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif //_WIN32

#include <string>
#include <assert.h>

#include "console.h"
#include "cinematics/fade_screen.h"
#include "label.h"
#include "menu.h"
#include "game_controller.h"
#include "asset_library.h"
#include "switchboard.h"
#include "object_3d.h"
#include "render_context.h"

//TODO: framerate counter

#define SDL_GAME_NUM_FPS_FRAMES   10

#define SDL_GAME_DEFAULT_WIDTH    640
#define SDL_GAME_DEFAULT_HEIGHT   480

#define SDL_GAME_GENERATE_PAUSE_MENU  (1<<0)
#define SDL_GAME_LOCK_SIM_DT          (1<<1)

#define SDL_GAME_STATE_PAUSED         (1<<0)
#define SDL_GAME_STATE_FULLSCREEN     (1<<1)

class GameContext
{
  friend class SDLGame;
public:
  Game::Switchboard        switch_board;
  Game::AssetLibrary       asset_library;
  DebugConsole             console;
  Graphics::RenderContext  render_context;

  SDL_Window               *sdl_window;
  SDL_GLContext            *sdl_gl_context;

  std::vector<Game::Object3D *> active_game_objects;

  GameContext() { sdl_window = NULL; sdl_gl_context = NULL; }
  ~GameContext() {}
};


class SDLGame
{
protected:
  Game::GameControllerContext game_controller_context;
  GameContext game_context;

  UI::WidgetWrangler      ww;

  uint32_t         flags;

  bool             recording_movie;
  int              movie_frame_counter;

  int              resolution[2];
  SDL_Window       *win;
  SDL_GLContext    gl_context;
  int              gl_version[2];
  int              gl_context_profile;
  std::string      window_title;

  double           last_game_time;

  std::string      font_face;
  Graphics::Font   *font;    //font we want to use for mostly everything
  unsigned int     font_size;

  std::string      widget_font_face;
  Graphics::Font   *widget_font;
  unsigned int     widget_font_size;

  int              fps_idx;
  float            prev_fps[SDL_GAME_NUM_FPS_FRAMES];
  UI::Label        fps_label;

  unsigned int     game_state; //paused, full-screen, etc...

  UI::Menu         *pause_menu;
  FadeScreen       title_screen;

  float            sim_lock_dt;
  bool             vsync_enabled;
  bool             diy_window_swap;

  std::unordered_map<uint32_t, void(*)(const SDL_Event &e)> ui_callback_map;

public:
  //SDLGame();
  SDLGame(const int w = SDL_GAME_DEFAULT_WIDTH,
          const int h = SDL_GAME_DEFAULT_HEIGHT,
          const std::string title = "Game",
          const unsigned int _flags = SDL_GAME_GENERATE_PAUSE_MENU | SDL_GAME_LOCK_SIM_DT,
          const int _gl_context_profile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY,
          const int gl_major_version = -1,
          const int gl_minor_version = -1);

  ~SDLGame();

  void set_gl_version(const int major, const int minor) { gl_version[0] = major; gl_version[1] = minor; }

  void set_resolution(const unsigned int w, const unsigned int h);
  void get_resolution(unsigned int &w, unsigned int &h) { w = resolution[0]; h = resolution[1]; }

  void toggle_fullscreen();
  void enable_vsync(const bool v) { vsync_enabled = v; }

  void init();
  void run();
  void process_events();

  void generate_ui_from_layout(std::string name);

  DebugConsole *get_debug_console() { return &game_context.console; }

  double get_game_time() const { return (double)SDL_GetTicks(); }

  void set_pause_menu(UI::Menu *menu) { assert(!(flags & SDL_GAME_GENERATE_PAUSE_MENU)); pause_menu = menu; }

  void set_main_font(std::string font_face_name, unsigned int size);
  void set_widget_font(std::string font_face_name, unsigned int size);

  void begin_video_capture() { recording_movie = true; movie_frame_counter = 0; }
  void end_video_capture() { recording_movie = false; movie_frame_counter = 0; }

  void quit_app();

protected:
  virtual void game_loop(const double game_time, const double frame_time) = 0;

  virtual void user_init() = 0;
  virtual void user_run() = 0;
  virtual void user_process_event(const SDL_Event &event) = 0;

  virtual void render_gl() = 0;

  virtual void init_sdl();

  void init_sdl_default();
  void init_sdl_gl_context();
  void set_sdl_attributes();
  void sdl_init_verbose();

  void screenshot();
};

#endif //__SDL_GAME_H__
