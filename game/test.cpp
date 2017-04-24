#include "sdl_game.h"

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

using namespace Math;

class MySDLGame : public SDLGame
{
public:
  MySDLGame() : SDLGame(640, 480, "Game Test", SDL_GAME_GENERATE_PAUSE_MENU | SDL_GAME_LOCK_SIM_DT) {}
  ~MySDLGame() {}

private:
  void game_loop(const double game_time, const double frame_time) {}
  void user_init()
  {
    float float_vars[5];
    bool bool_vars[3];
    Float3 float3_vars[2];

    game_context.console.register_variable(&float_vars[0], "apple_sauce");
    game_context.console.register_variable(&float_vars[1], "peanut_butter");
    game_context.console.register_variable(&float_vars[2], "app");
    game_context.console.register_variable(&float_vars[3], "flossy");
    game_context.console.register_variable(&float_vars[4], "razzle");

    game_context.console.register_variable(&bool_vars[0], "razzle2");
    game_context.console.register_variable(&bool_vars[1], "RAZZLE");
    game_context.console.register_variable(&bool_vars[2], "pig_snack");

    game_context.console.register_variable(&float3_vars[0], "mouse");
    game_context.console.register_variable(&float3_vars[1], "sauce");
    game_context.console.register_variable(&float3_vars[2], "flossy");
  }
  void user_run() {}
  void user_process_event(const SDL_Event &event) {}
  void render_gl()
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
};

int main(int argc, char **argv)
{
  MySDLGame game;
  game.init();

  game.run();
  return 0;
}
