#pragma once

#include <imgui.h>
#include <vector>

static char Default_file_filter[] = "All\0*.*\0Text\0*.TXT\0";

std::string open_file_dialog(std::string dir, char *filter = Default_file_filter);
std::string make_filename_relative(std::string fname);

namespace ImGui
{
  bool ListBox(const char* label, int* current_item, const std::vector<std::string>& items, int items_count, int height_in_items = -1);
  bool InputText(const char *label, std::string &text);
}