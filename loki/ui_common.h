#pragma once

#include <imgui.h>
#include <vector>

namespace ImGui
{
  bool ListBox(const char* label, int* current_item, const std::vector<std::string>& items, int items_count, int height_in_items = -1);
}