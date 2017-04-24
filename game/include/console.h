#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <vector>
#include <string>
#include <stdio.h>

#include "math_utility.h"
#include "font.h"
#include "widget_wrangler.h"
#include "meter.h"

#define DEFAULT_CONSOLE_FONT_SIZE   10

enum ConsoleState
{
  CONSOLE_INACTIVE,
  CONSOLE_ACTIVE_DEFAULT,
  CONSOLE_ACTIVE_CONTROL_BOARD
};

class ConsoleVariable
{
private:
  std::string name;
  void *data_ptr;
public:
  ConsoleVariable(std::string n) { name = n; }
  ~ConsoleVariable() {}
};

class ConsoleVariableFloat : public ConsoleVariable
{
private:
public:
  ConsoleVariableFloat(std::string n) : ConsoleVariable(n) {}
  ~ConsoleVariableFloat() {}
};

//logging / console output
struct ConsoleLog : std::ostream, std::streambuf
{
  ConsoleLog() : std::ostream(this) { max_display_lines = 10; log_file = NULL; }

  int overflow(int c)
  {
    handle_char(c);
    return 0;
  }

  void handle_char(char c)
  {
    buffer = buffer + c;

    if (log_file)
    {
      fwrite(&c, sizeof(char), 1, log_file);
    }

    //count lines
    uint32_t num_lines = 0;
    for (uint32_t i = 0; i < buffer.size(); i++)
    {
      num_lines = buffer[i] == '\n' ? num_lines + 1 : num_lines;
    }

    uint32_t counter = 0;
    while (num_lines > max_display_lines)
    {
      if (buffer[counter] == '\n')
      {
        buffer.erase(buffer.begin(), buffer.begin() + counter + 1);

        counter = 0;
        num_lines--;
      }
      else
      {
        counter++;
      }
    }
    buffer.shrink_to_fit();
  }

  //void set_text_color(const Math::Float3 c) { text_color = c; }
  std::string buffer;
  uint32_t max_display_lines;

  FILE *log_file;
};

class DebugConsole
{
private:
  float                       pct_exposed;
  ConsoleState                state;

  Graphics::Font              *font;
  Math::Float3                bg_color;
  float                       bg_opacity;
  Math::Float3                text_color;

  std::string                 current_command;
  std::string                 console_buffer;

  int                         command_history_idx;
  std::vector<std::string>    command_history;

  std::vector<std::string>    boolean_var_names;
  std::vector<bool *>         boolean_vars;

  std::vector<std::string>    float_var_names;
  std::vector<float *>        float_vars;
  std::vector<Math::Float2 >  float_var_ranges;
  std::vector<UI::Meter *>    float_var_sliders;

  std::vector<std::string>    float2_var_names;
  std::vector<Math::Float2 *> float2_vars;

  std::vector<std::string>    float3_var_names;
  std::vector<Math::Float3 *> float3_vars;

  std::vector<std::string>    func_var_names;
  std::vector<void(*)()>      func_vars;

  //tab completion
  std::string                 tab_complete_string;
  int32_t                     last_tab_complete_idx;

  UI::WidgetWrangler          console_ww;
  float                       control_board_scroll;

public:
  DebugConsole();
  ~DebugConsole();

  ConsoleLog console_log; //debug output, etc.

  void init();
  void activate(ConsoleState s);
  bool is_active() const;
  bool is_control_board_active() const;
  void receive_char(const char c);
  void backspace();
  void execute();
  void simulate(const double game_time, const double frame_time);
  void process_event(const SDL_Event &e);

  void print_line(std::string s);

  void render_gl();
  void render_console_log();
  void render_default();
  void render_control_board();

  void set_bg_color(Math::Float3 c) { bg_color = c; }
  void set_text_color(Math::Float3 c) { text_color = c; }

  void read_param_file();
  void write_param_file();

  void register_variable(bool *b, const char *name);
  void register_variable(float *f, const char *name, const Math::Float2 range = Math::Float2(0.0f, 1.0f));
  void register_variable(Math::Float2 *f, const char *name);
  void register_variable(Math::Float3 *f, const char *name);
  void register_function(void (*f)(), const char *name);

  void traverse_command_history(const int dir);
  void tab_complete(int depth = 0);

	void set_font(Graphics::Font *f) { font = f; }
};

#endif // __CONSOLE_H__
