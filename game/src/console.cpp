#include "console.h"

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "math_utility.h"

#include "mxml.h"

#include "meter.h"
#include "label.h"

#define EXPOSE_PERCENT_RATE 5.0f

using namespace Math;
using namespace UI;
using namespace std;

DebugConsole::DebugConsole()
{
  font = NULL;
  pct_exposed = 0.0f;
  state = CONSOLE_INACTIVE;

  command_history_idx = 0;

  bg_color = Float3(0.6f, 0.4f, 1.0f);
  float3_var_names.push_back("console.bg_color");
  float3_vars.push_back(&bg_color);

  bg_opacity = 0.55f;
  float_var_names.push_back("console.bg_opacity");
  float_vars.push_back(&bg_opacity);
  float_var_ranges.push_back(Float2(0.0f, 1.0f));

  text_color = Float3(0.0f, 0.0f, 0.0f);
  float3_var_names.push_back("console.text_color");
  float3_vars.push_back(&text_color);

  last_tab_complete_idx = -1;

  control_board_scroll = 0.0f;

  FILE *log_file = fopen("console.log", "w");
  assert(log_file);
  console_log.log_file = log_file;
}

DebugConsole::~DebugConsole()
{
  if (console_log.log_file)
  {
    fclose(console_log.log_file);
  }
}

void DebugConsole::activate(ConsoleState s)
{
  state = s;
}


bool DebugConsole::is_active() const
{
  if((state == CONSOLE_ACTIVE_DEFAULT))
  {
    return true;
  }
  else if(pct_exposed < 0.01f)
  {
    return false;
  }
  return true;
}

bool DebugConsole::is_control_board_active() const
{
  return (state == CONSOLE_ACTIVE_CONTROL_BOARD);
}

void DebugConsole::init()
{
  current_command.clear();
  command_history.clear();
  tab_complete_string.clear();
  last_tab_complete_idx = -1;

  //initialized the control board
  float v_offset = 120.0f;
  float h_offset = 50.0f;
  for (unsigned int i = 0; i < float_vars.size(); i++)
  {
    Meter *m = new Meter;
    m->translate(Float2(h_offset, v_offset));
    m->scale(Float2(100.0f, 20.0f));
    m->init();
    m->show();
    m->set_percent(0.0f);

    console_ww.add_widget(m);
    float_var_sliders.push_back(m);

    Label *l = new Label;
    l->set_font(font);
    l->translate(Float2(h_offset + 120.0f, v_offset - 5.0f));
    l->scale(Float2(500.0f, 20.0f));
    l->set_text(float_var_names[i]);
    l->init();
    l->show();
    console_ww.add_widget(l);

    v_offset += 30.0f;
  }
}

void DebugConsole::simulate(const double game_time, const double frame_time)
{
  if(state == CONSOLE_ACTIVE_DEFAULT)
  {
    pct_exposed += (float)frame_time * EXPOSE_PERCENT_RATE;
  }
  else
  {
    pct_exposed -= (float)frame_time * EXPOSE_PERCENT_RATE;
  }
  pct_exposed = Math::clamp(pct_exposed, 0.0f, 1.0f);

  //if (state == CONSOLE_ACTIVE_CONTROL_BOARD)
  {
    console_ww.simulate(game_time, frame_time);
    for (unsigned int i = 0; i < float_vars.size(); i++)
    {
      Meter *m = float_var_sliders[i];
      Float2 range = float_var_ranges[i];
      float *val = float_vars[i];

      float pct = (*val - range[0]) / (range[1] - range[0]);
      m->set_percent(pct);
    }
  }
}

void DebugConsole::process_event(const SDL_Event &e)
{
  console_ww.process_event(e, Float2(0.0f, -control_board_scroll));

  //gather data
  for (unsigned int i = 0; i < float_var_sliders.size(); i++)
  {
    Meter *m = float_var_sliders[i];
    Float2 range = float_var_ranges[i];
    float val = range[0] + m->get_percent() * (range[1] - range[0]);
    float *target = float_vars[i];
    *target = val;
  }

  //handle control board scrolling
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  if(keystate[SDL_SCANCODE_UP])
  {
    control_board_scroll -= 10.0f;
  }
  if (keystate[SDL_SCANCODE_DOWN])
  {
    control_board_scroll += 10.0f;
  }
}

void DebugConsole::receive_char(const char c)
{
  switch(c)
  {
    case 17:
    case 18:
      //traverse_command_history(c);
      break;
    default:
      current_command.push_back(c);
      break;
  }
  tab_complete_string = current_command;
  last_tab_complete_idx = -1;
}

void DebugConsole::backspace()
{
  if(current_command.size() > 0)
  {
    current_command.pop_back();
  }
  tab_complete_string = current_command;
  last_tab_complete_idx = -1;
}

//TODO: rewrite. this is shit.
void DebugConsole::execute()
{
  //split string by spaces
  std::vector<std::string> words;
  boost::split(words, current_command, boost::is_any_of(" "), boost::token_compress_on);

  //search single float variables
  std::vector<string>::iterator i;
  i = std::find(float_var_names.begin(), float_var_names.end(), words[0]);
  if(i != float_var_names.end())
  {
    int idx = (int)std::distance(float_var_names.begin(), i);
    float *f = float_vars[idx];
    if(words.size() == 1 || (words.size() == 2 && words[1] == ""))
    {
      //just output the current value
      cout<<float_var_names[idx].c_str()<<": "<<*f<<endl;
      print_line(float_var_names[idx].c_str() + std::string(": ") + std::to_string(*f));
    }
    else if(words.size() == 2)
    {
      //cout<<"idx: "<<idx<<endl;
      float val = (float)atof(words[1].c_str());
      *f = val;
    }
  }

  //search float2 variables
  i = std::find(float2_var_names.begin(), float2_var_names.end(), words[0]);
  if (i != float2_var_names.end())
  {
    int idx = std::distance(float2_var_names.begin(), i);
    Float2 *f = float2_vars[idx];
    if (words.size() == 1 || (words.size() == 2 && words[1] == ""))
    {
      /*
      std::ostringstream ss;
      ss<<*f;
      std::string float_string(ss.str());
      */
      cout << float2_var_names[idx].c_str() << ": " << *f << endl;
      print_line(float2_var_names[idx] + std::string(": ") + std::to_string((*f)[0]) + std::to_string((*f)[1]));
    }
    else if (words.size() == 3)
    {
      float val[2];
      val[0] = (float)atof(words[1].c_str());
      val[1] = (float)atof(words[2].c_str());

      *f = Float2(val[0], val[1]);
    }
  }

  //search float3 variables
  i = std::find(float3_var_names.begin(), float3_var_names.end(), words[0]);
  if(i != float3_var_names.end())
  {
    int idx = std::distance(float3_var_names.begin(), i);
    Float3 *f = float3_vars[idx];
    if(words.size() == 1 || (words.size() == 2 && words[1] == ""))
    {
      /*
      std::ostringstream ss;
      ss<<*f;
      std::string float_string(ss.str());
      */
      cout<<float3_var_names[idx].c_str()<<": "<<*f<<endl;
      print_line(float3_var_names[idx] + std::string(": ") + std::to_string((*f)[0]) + std::to_string((*f)[1]) + std::to_string((*f)[2]));
    }
    else if(words.size() == 4)
    {
      float val[3];
      val[0] = (float)atof(words[1].c_str());
      val[1] = (float)atof(words[2].c_str());
      val[2] = (float)atof(words[3].c_str());

      *f = Float3(val[0], val[1], val[2]);
    }
  }

  //search boolean variables
  i = std::find(boolean_var_names.begin(), boolean_var_names.end(), words[0]);
  if(i != boolean_var_names.end())
  {
    int idx = (int)std::distance(boolean_var_names.begin(), i);
    bool *b = boolean_vars[idx];

    if(words.size() == 2)
    {
      bool val = (atoi(words[1].c_str()) != 0);
      *b = val;
    }
    else if(words.size() == 1)
    {
      //*b = !(*b);
      cout<<boolean_var_names[idx].c_str()<<": ";
      if(*b) { cout<<"true"; } else { cout<<"false"; }
      cout<<endl;
    }
  }

  i = std::find(func_var_names.begin(), func_var_names.end(), words[0]);
  if(i != func_var_names.end())
  {
    int idx = (int)std::distance(func_var_names.begin(), i);
    void (*f)() = func_vars[idx];
    f();
  }

  command_history.push_back(current_command);
  command_history_idx = (int)command_history.size();
  //console_buffer += "\n" + current_command;
  print_line(current_command);
  current_command.clear();
  tab_complete_string.clear();
  last_tab_complete_idx = -1;
}

void DebugConsole::print_line(std::string s)
{
  console_buffer = s + std::string("\n") + console_buffer;
}

void DebugConsole::render_gl()
{
  switch (state)
  {
  case CONSOLE_ACTIVE_DEFAULT:
    render_default();
    break;
  case CONSOLE_ACTIVE_CONTROL_BOARD:
    render_control_board();
    break;
  default:
    if (pct_exposed > 0.01f) { render_default();  }
    render_console_log();
    break;
  }
}

void DebugConsole::render_console_log()
{
  //display console log
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  float v_pixels = (float)viewport[3] - 0.25f * (float)viewport[3];
  std::vector<std::string> lines;
  boost::split(lines, console_log.buffer, boost::is_any_of("\n"), boost::token_compress_on);
  float v_h = font->get_height() + 5.0f;
  for (unsigned int i = 0; i < lines.size(); i++)
  {
    glColor4f(0.0f, 0.5f, 0.5f, 0.7f);
    font->print(10, v_pixels + 15.0f + i * v_h, lines[i].c_str());
  }
}

void DebugConsole::render_control_board()
{
  //glTranslatef(0.0f, control_board_scroll, 0.0f);
  console_ww.render(Float3(0.0f, control_board_scroll, 0.0f));
}

void DebugConsole::render_default()
{
  //animation
  float v_offset = Math::cerp(-0.5f, 0.0f, pct_exposed);

  glUseProgram(0);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //first render the backdrop
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0f, v_offset, 0.0f);
  glColor4f(bg_color[0], bg_color[1], bg_color[2], bg_opacity);

  //TODO: VBO rendering
  glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.4f);              // Top Left
    glVertex2f( 1.0f, -0.4f);              // Top Right
    glVertex2f( 1.0f, -1.0f);              // Bottom Right
    glVertex2f(-1.0f, -1.0f);              // Bottom Left
  glEnd();

  //now render the text
  glColor3f(text_color[0], text_color[1], text_color[2]);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0f, v_offset * 256, 0.0f);

  static char command_line[1024];
  sprintf(command_line, "your wish > %s", current_command.c_str());

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  float v_pixels = (float)viewport[3] - 0.25f * (float)viewport[3];
  font->print(10, v_pixels, command_line);

  //can't print the whole thing at once (buffer overflow), so do it line by line
  //split string by carriage returns
  std::vector<std::string> lines;
  boost::split(lines, console_buffer, boost::is_any_of("\n"), boost::token_compress_on);
  float v_h = font->get_height() + 5.0f;
  for(unsigned int i = 0; i < lines.size(); i++)
  {
    font->print(10, v_pixels + 15.0f + i * v_h, lines[i].c_str());
  }
}

void DebugConsole::read_param_file()
{
  FILE *f = fopen("params.xml", "r");
  if(f)
  {
    mxml_node_t *tree = mxmlLoadFile(NULL, f, MXML_TEXT_CALLBACK);
    assert(tree);
    
    mxml_node_t *node = tree;
    do
    {
      node = mxmlFindElement(node, tree, "float_var", NULL, NULL, MXML_DESCEND);
      if(node)
      {
        const char *var_name, *var_value;
        var_name = mxmlElementGetAttr(node, "name");
        var_value = mxmlGetText(node, NULL);
        
        current_command = string(var_name) + string(" ") + string(var_value);
        console_log << current_command << endl;
        execute();
        
      }
    } while(node);
    
    fclose(f);
  }
}

//
// sipping swigs of Caol Ila scotch whisky while driving through the Scottish highlands
// after a lovely day on the Isle of Skye
//
void DebugConsole::write_param_file()
{
  FILE *f = fopen("params.xml", "w");
  
  console_log << "writing game parameter file..."<<endl;
  if(f)
  {
    console_log << float_var_names.size() << "variable names"<<endl;
    
    string header = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\" ?>\n\n";
    fwrite(header.c_str(), header.size(), sizeof(char), f);
    for(uint32_t i = 0; i < float_var_names.size(); i++)
    {
      std::stringstream ss;
      string label = "<float_var name=\"";
      fwrite(label.c_str(), label.size(), sizeof(char), f);
      fwrite(float_var_names[i].c_str(), float_var_names[i].size(), sizeof(char), f);
      label = "\">";
      fwrite(label.c_str(), label.size(), sizeof(char), f);
    
      ss<<std::fixed<<*(float_vars[i]);
      string value = ss.str();
      fwrite(value.c_str(), value.size(), sizeof(char), f);
    
      label = "</float_var>\n";
      fwrite(label.c_str(), label.size(), sizeof(char), f);
    }
    fclose(f);
  }
  else
  {
    console_log << "Could not open file for writing!"<<endl;
  }
}

void DebugConsole::register_variable(bool *b, const char *name)
{
  std::string n(name);
  boolean_var_names.push_back(n);
  boolean_vars.push_back(b);
}

void DebugConsole::register_variable(float *f, const char *name, const Float2 range)
{
  std::string n(name);
  float_var_names.push_back(n);
  float_vars.push_back(f);
  float_var_ranges.push_back(range);
}

void DebugConsole::register_variable(Float2 *f, const char *name)
{
  std::string n(name);
  float2_var_names.push_back(n);
  float2_vars.push_back(f);
}

void DebugConsole::register_variable(Float3 *f, const char *name)
{
  std::string n(name);
  float3_var_names.push_back(n);
  float3_vars.push_back(f);
}

void DebugConsole::register_function(void (*f)(), const char *name)
{
  std::string n(name);
  func_var_names.push_back(n);
  func_vars.push_back(f);
}

void DebugConsole::traverse_command_history(const int dir)
{
  if(command_history.size() > 0)
  {
    //current_command = command_history[command_history_idx];
    command_history_idx = (dir > 0) ? command_history_idx - 1 : command_history_idx + 1;
    if(command_history_idx < 0)
    {
      command_history_idx = 0;
    }
    if(command_history_idx >= (int)command_history.size())
    {
      command_history_idx = (int)command_history.size() - 1;
    }
    current_command = command_history[command_history_idx];
  }
}

//everything about this function is horrible
void DebugConsole::tab_complete(int depth)
{
  if(tab_complete_string.size() <= 0 || last_tab_complete_idx < -1)
  {
    return;
  }

  if((last_tab_complete_idx + 1) < (int32_t)float_var_names.size())
  {
    for(int32_t i = last_tab_complete_idx + 1; i < (int32_t)float_var_names.size(); i++)
    {
      if(float_var_names[i].find(tab_complete_string) != std::string::npos)
      {
        current_command = float_var_names[i] + " ";
        last_tab_complete_idx = i;
        return;
      }
    }
  }

  //float2 variables
  int idx_offset = float_var_names.size();
  if(last_tab_complete_idx < idx_offset)
  {
    last_tab_complete_idx = idx_offset;
  }

  for(int32_t i = last_tab_complete_idx - idx_offset; i < (int32_t)float2_var_names.size(); i++)
  {
    if(float2_var_names[i].find(tab_complete_string) != std::string::npos)
    {
      current_command = float2_var_names[i] + " ";
      last_tab_complete_idx = i + idx_offset + 1;
      return;
    }
  }

  //float3
  idx_offset = float2_var_names.size() + float_var_names.size();
  if (last_tab_complete_idx < idx_offset)
  {
    last_tab_complete_idx = idx_offset;
  }
  for (int32_t i = last_tab_complete_idx - idx_offset; i < (int32_t)float3_var_names.size(); i++)
  {
    if (float3_var_names[i].find(tab_complete_string) != std::string::npos)
    {
      cout << "i:" << i << endl;
      current_command = float3_var_names[i] + " ";
      last_tab_complete_idx = i + idx_offset + 1;
      return;
    }
  }

  //boolean variables
  idx_offset = float3_var_names.size() + float2_var_names.size() + float_var_names.size();
  if(last_tab_complete_idx < idx_offset)
  {
    last_tab_complete_idx = idx_offset;
  }
  for(int32_t i = last_tab_complete_idx - idx_offset; i < (int32_t)boolean_var_names.size(); i++)
  {
    if(boolean_var_names[i].find(tab_complete_string) != std::string::npos)
    {
      cout<<"i:"<<i<<endl;
      current_command = boolean_var_names[i] + " ";
      last_tab_complete_idx = i + idx_offset + 1;
      return;
    }
  }

  //functions
  idx_offset += boolean_var_names.size();
  if(last_tab_complete_idx < idx_offset)
  {
    last_tab_complete_idx = idx_offset;
  }
  for(int32_t i = last_tab_complete_idx - idx_offset; i < (int32_t)func_var_names.size(); i++)
  {
    if(func_var_names[i].find(tab_complete_string) != std::string::npos)
    {
      current_command = func_var_names[i] + " ";
      last_tab_complete_idx = i + func_var_names.size() + 1;
      return;
    }
  }

  //didn't find anything.... :(
  last_tab_complete_idx = -1;
  if(depth == 0) { tab_complete(depth + 1); }
}
