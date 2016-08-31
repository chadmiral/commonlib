#include <iostream>
#include "layout.h"
#include "platform.h"
#include "toolbox.h"
#include "check_button.h"
#include "radio_group.h"

using namespace UI;
using namespace std;
using namespace Math;

const char *get_xml_opaque_safe(mxml_node_t *parent, mxml_node_t *root, const char *tag_name)
{
  mxml_node_t *n = mxmlFindElement(parent, root, tag_name, NULL, NULL, MXML_NO_DESCEND);
  if (n)
  {
    const char *buffer = mxmlGetOpaque(n);
    if (buffer)
    {
      cout << "\t" << tag_name << ": " << buffer << endl;
      return buffer;
    }
  }
  return NULL;
}

void parse_layout_template_xml(LayoutWidgetTemplate *wt, mxml_node_t *widget_node, mxml_node_t *root, LayoutWidgetTemplate *parent, LayoutWidgetTemplate *radio_group)
{
  const char *buffer = NULL;

  buffer = mxmlElementGetAttr(widget_node, "name");
  if (buffer)
  {
    cout << "\tname: " << buffer << endl;
  }

  uint32_t x = 100;
  uint32_t y = 100;
  uint32_t width = 32;
  uint32_t height = 32;
  buffer = mxmlElementGetAttr(widget_node, "width");
  if (buffer) { width = atoi(buffer); }
  buffer = mxmlElementGetAttr(widget_node, "height");
  if (buffer) { height = atoi(buffer); }
  buffer = mxmlElementGetAttr(widget_node, "x");
  if (buffer) { x = atoi(buffer); }
  buffer = mxmlElementGetAttr(widget_node, "y");
  if (buffer) { y = atoi(buffer); }
  buffer = mxmlElementGetAttr(widget_node, "align_x");
  if (buffer && !stricmp(buffer, "center")) { wt->flags |= UI_LAYOUT_FLAG_ALIGN_CENTER_X; }
  if (buffer && !stricmp(buffer, "right")) { wt->flags |= UI_LAYOUT_FLAG_ALIGN_RIGHT; }
  buffer = mxmlElementGetAttr(widget_node, "align_y");
  if (buffer && !stricmp(buffer, "bottom")) { wt->flags |= UI_LAYOUT_FLAG_ALIGN_BOTTOM; }
  if (buffer && !stricmp(buffer, "center")) { wt->flags |= UI_LAYOUT_FLAG_ALIGN_CENTER_Y; }
  buffer = mxmlElementGetAttr(widget_node, "orientation");
  if (buffer && !stricmp(buffer, "vertical")) { wt->flags |= UI_LAYOUT_FLAG_VERTICAL; }
  
  cout << "\tdim: (" << width << ", " << height << ")" << endl;
  cout << "\toffset: " << x << ", " << y << ")" << endl;
  wt->dim = Float2((float)width, (float)height);
  wt->offset = Float2((float)x, (float)y);

  mxml_node_t *child_node = widget_node->child;
  if (child_node)
  {
    //tool tip
    buffer = get_xml_opaque_safe(child_node, root, "tool_tip");
    if (buffer) { wt->tool_tip = buffer; }

    //default texture
    buffer = get_xml_opaque_safe(child_node, root, "tex_default");
    if (buffer) { wt->tex_default = buffer; }

    //active texture
    buffer = get_xml_opaque_safe(child_node, root, "tex_active");
    if (buffer) { wt->tex_active = buffer; }

    //"off" texture
    buffer = get_xml_opaque_safe(child_node, root, "tex_off");
    if (buffer) { wt->tex_off = buffer; }

    //callback function
    buffer = get_xml_opaque_safe(child_node, root, "callback");
    if (buffer) { wt->click_callback = buffer; }
  }

  if (parent)
  {
    cout << "\tparent detected..." << endl;
    parent->children.push_back(wt);
    wt->parent = parent;
  }

  if (radio_group)
  {
    cout << "\tradio group detected..." << endl;
    radio_group->children.push_back(wt);
  }
}

void Layout::parse_xml_level(mxml_node_t *root, LayoutWidgetTemplate *parent, LayoutWidgetTemplate *radio_group)
{
  mxml_node_t *start_node = root->child;
  const char *buffer = NULL;

  //parse toolboxes
  mxml_node_t *toolbar_node = NULL;
  do
  {
    //use MXML_NO_DESCEND so we stay at the top level
    toolbar_node = mxmlFindElement(start_node, root, "toolbar", NULL, NULL, MXML_NO_DESCEND);
    if (toolbar_node)
    {
      //cout << "adding toolbar..." << endl;
      //ToolBox *tb = new ToolBox;
      LayoutWidgetTemplate *tb = new LayoutWidgetTemplate;
      tb->type = WIDGET_TOOLBOX;

      parse_layout_template_xml(tb, toolbar_node, root, parent, radio_group);

      /*
      buffer = mxmlElementGetAttr(toolbar_node, "name");
      cout << "\tname: " << buffer << endl;

      buffer = mxmlElementGetAttr(toolbar_node, "orientation");
      if (buffer)
      {
        cout << "\torientation: " << buffer << endl;
        if (!stricmp(buffer, "vertical")) { tb->flags |= UI_LAYOUT_FLAG_VERTICAL; }
      }

      buffer = mxmlElementGetAttr(toolbar_node, "x");
      uint32_t x_offset = atoi(buffer);
      buffer = mxmlElementGetAttr(toolbar_node, "y");
      uint32_t y_offset = atoi(buffer);

      cout << "\toffset: (" << x_offset << ", " << y_offset << ")" << endl;

      tb->offset = Float2((float)x_offset, (float)y_offset);
      */

      templates.push_back(tb);
      parse_xml_level(toolbar_node, tb);
    }
    start_node = toolbar_node;
  } while (toolbar_node);

  //parse radio groups
  start_node = root->child;
  mxml_node_t *radio_group_node = NULL;
  do
  {
    radio_group_node = mxmlFindElement(start_node, root, "radio_group", NULL, NULL, MXML_NO_DESCEND);
    if (radio_group_node)
    {
      //cout << "adding radio group" << endl;
      LayoutWidgetTemplate *rg = new LayoutWidgetTemplate;
      rg->type = WIDGET_RADIO_GROUP;

      if (parent)
      {
        //cout << "parent detected..." << endl;
        parent->children.push_back(rg);
      }

      templates.push_back(rg);
      parse_xml_level(radio_group_node, parent, rg);
    }
    start_node = radio_group_node;
  } while (radio_group_node);

  //parse check button widgets
  start_node = root->child;
  mxml_node_t *check_button_node = NULL;
  do
  {
    check_button_node = mxmlFindElement(start_node, root, "check_button", NULL, NULL, MXML_NO_DESCEND);
    if (check_button_node)
    {
      //cout << "adding checkbutton..." << endl;
      LayoutWidgetTemplate *cb = new LayoutWidgetTemplate;
      cb->type = WIDGET_CHECK_BUTTON;

      parse_layout_template_xml(cb, check_button_node, root, parent, radio_group);

      templates.push_back(cb);
    }
    start_node = check_button_node;
  } while (check_button_node);

  //parse push button widgets
  start_node = root->child;
  mxml_node_t *push_button_node = NULL;
  do
  {
    push_button_node = mxmlFindElement(start_node, root, "push_button", NULL, NULL, MXML_NO_DESCEND);
    if (push_button_node)
    {
      //cout << "adding pushbutton..." << endl;
      LayoutWidgetTemplate *pb = new LayoutWidgetTemplate;
      pb->type = WIDGET_PUSH_BUTTON;

      parse_layout_template_xml(pb, push_button_node, root, parent, radio_group);

      templates.push_back(pb);
    }
    start_node = push_button_node;
  } while (push_button_node);
}

void Layout::read_from_xml(mxml_node_t *root, uint32_t version)
{
  //cout << "loading UI layout from xml..." << endl;
  //cout << "\tversion: " << version << endl;

  mxml_node_t *layout_node = mxmlFindElement(root, root, "ui_layout", NULL, NULL, MXML_DESCEND);
  if (layout_node)
  {
    parse_xml_level(layout_node);
  }
  else
  {
    SET_TEXT_COLOR(CONSOLE_COLOR_RED);
    cout << "Could not find root ui_layout node!" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_RED);
  }
}