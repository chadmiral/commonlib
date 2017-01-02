#ifndef __TOOL_H__
#define __TOOL_H__

#include <assert.h>
#include <mxml.h>
#include "math_utility.h"

namespace Tool
{
  std::string mxml_read_text(mxml_node_t *node);
  Math::Float2 mxml_read_float2(const mxml_node_t *node);
  Math::Float3 mxml_read_float3(const mxml_node_t *node);

  std::string make_nice_filename(std::string prefix, std::string suffix, int counter = 0);
}

#endif //__ASSET_TYPES_H__
