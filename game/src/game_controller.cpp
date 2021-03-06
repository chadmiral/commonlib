#include <iostream>
#include <assert.h>
#include "game_controller.h"

using namespace Game;
using namespace std;

void GameControllerContext::init()
{
  int num_joysticks = SDL_NumJoysticks();
  cout << "GameControllerContext::init(): " << num_joysticks << " game controller detected." << endl;
  int controller_index = 0;
  for (int joy = 0; joy < num_joysticks; ++joy)
  {
    if (!SDL_IsGameController(joy))
    {
      continue;
    }
    if (controller_index >= MAX_GAME_CONTROLLERS)
    {
      break;
    }
    controllers[controller_index] = SDL_GameControllerOpen(joy);
    controller_index++;
  }
}

void GameControllerContext::deinit()
{
  for (int ci = 0; ci < MAX_GAME_CONTROLLERS; ++ci)
  {
    if (controllers[ci])
    {
      SDL_GameControllerClose(controllers[ci]);
    }
  }
}

void GameControllerContext::poll_input(const unsigned int ci, GameControllerState *state)
{
  assert(ci < MAX_GAME_CONTROLLERS);

 // for (int ci = 0; ci < MAX_GAME_CONTROLLERS; ci++)
  //{
    if (controllers[ci] && SDL_GameControllerGetAttached(controllers[ci]))
    {
      // NOTE: We have a controller with index ci.
      state->up = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
      state->down = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
      state->left = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
      state->right = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
      state->start = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_START) != 0;
      state->back = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_BACK) != 0;
      state->leftShoulder = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;
      state->rightShoulder = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;
      state->aButton = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_A) != 0;
      state->bButton = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_B) != 0;
      state->xButton = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_X) != 0;
      state->yButton = SDL_GameControllerGetButton(controllers[ci], SDL_CONTROLLER_BUTTON_Y) != 0;

      //analog sticks
      state->stick_lx = (float)SDL_GameControllerGetAxis(controllers[ci], SDL_CONTROLLER_AXIS_LEFTX) / GAME_CONTROLLER_AXIS_RANGE;
      state->stick_ly = (float)SDL_GameControllerGetAxis(controllers[ci], SDL_CONTROLLER_AXIS_LEFTY) / GAME_CONTROLLER_AXIS_RANGE;
      state->stick_rx = (float)SDL_GameControllerGetAxis(controllers[ci], SDL_CONTROLLER_AXIS_RIGHTX) / GAME_CONTROLLER_AXIS_RANGE;
      state->stick_ry = (float)SDL_GameControllerGetAxis(controllers[ci], SDL_CONTROLLER_AXIS_RIGHTY) / GAME_CONTROLLER_AXIS_RANGE;

      //triggers
      state->trigger_l = (float)SDL_GameControllerGetAxis(controllers[ci], SDL_CONTROLLER_AXIS_TRIGGERLEFT) / GAME_CONTROLLER_AXIS_RANGE;
      state->trigger_r = (float)SDL_GameControllerGetAxis(controllers[ci], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / GAME_CONTROLLER_AXIS_RANGE;

  
      /*
      cout << "Left Stick: (" << state->stick_lx << ", " << state->stick_ly << ")" << endl;
      cout << "Right Stick: (" << state->stick_rx << ", " << state->stick_ry << ")" << endl;
      cout << "Left Trigger: " << state->trigger_l << endl;
      cout << "Right Trigger: " << state->trigger_r << endl;
      */
 
    }
    else
    {
      // TODO: This controller is note plugged in.
    }
  //}
}