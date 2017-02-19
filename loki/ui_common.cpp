#include <iostream>
#include <windows.h>
#include "ui_common.h"

using namespace std;

std::string open_file_dialog(std::string dir, char *filter)
{
  char szFile[100];
  OPENFILENAME ofn;

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = dir.c_str();
  ofn.Flags = 0;// OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);

  std::string fname(ofn.lpstrFile);

  //replace backslashes with front-slashes
  while (fname.find('\\') != std::string::npos)
  {
    size_t idx = fname.find('\\');
    fname.replace(idx, 1, "/");
  }

  return fname;
}

string make_filename_relative(string fname)
{
  size_t idx = fname.find("/data/");
  if (idx != std::string::npos)
  {
    return fname.substr(idx + strlen("/data/"));
  }
  return fname;
}

bool ImGui::ListBox(const char* label, int* current_item, const std::vector<std::string>& items, int items_count, int height_in_items)
{
  return ListBox(label, current_item, [](void* data, int idx, const char** out_text)
  {
    *out_text = (*((std::vector<std::string>*)data))[idx].c_str();
    return true;
  }, (void*)&items, items_count, height_in_items);
}

/*
bool ImGui::InputText(const char *label, std::string &text)
{
  static char buffer[256];
  return InputText(label, buffer, 256, ImGuiInputTextFlags_CallbackCharFilter,
    [](ImGuiTextEditCallbackData *data)
    {
      std::string *str = (std::string *)data->UserData;
      std::string new_char(1, data->EventChar);
      (*str) = (*str) + new_char;
      cout << str->c_str() << endl;
      return 0;
    },
    &text);
}*/