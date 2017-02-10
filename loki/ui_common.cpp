#include "ui_common.h"

bool ImGui::ListBox(const char* label, int* current_item, const std::vector<std::string>& items, int items_count, int height_in_items)
{
  return ListBox(label, current_item, [](void* data, int idx, const char** out_text)
  { *out_text = (*((std::vector<std::string>*)data))[idx].c_str();
  return true; }, (void*)&items, items_count, height_in_items);
}