#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <iostream>

#include "platform_gl.h"
#include "platform_sdl.h"

#include "math_utility.h"
#include "font.h"

namespace UI
{
  enum WidgetType
  {
    //containers
    WIDGET_TOOLBOX,
    WIDGET_RADIO_GROUP,
    
    //true widgets
    WIDGET_LABEL,
    WIDGET_TEXT_EDIT,
    WIDGET_PUSH_BUTTON,
    WIDGET_RADIAL_PUSH_BUTTON,
    WIDGET_CHECK_BUTTON,
    WIDGET_CURVE_EDITOR,
    WIDGET_MESSAGE_BOX,
    WIDGET_METER,

    WIDGET_MENU
  };

  class Widget
  {
  protected:
    WidgetType widget_type;

    void render_prep();
    void render_cleanup();

    bool visible;
    Graphics::Font *font;

    bool has_focus;

    bool has_tooltip;
    std::string tooltip;
    Math::Float2 tooltip_offset;

    bool hovering;
    float hover_timer;

    bool enabled;

  public:
    Widget(WidgetType t, Graphics::Font *f = NULL);
    ~Widget() {}

    WidgetType get_widget_type() const { return widget_type; }

    virtual void show(const bool d = true) { visible = d; }
    bool is_visible() const { return visible; }
    void set_font(Graphics::Font *f) { font = f; }

    virtual bool hit_test(const int x, const int y) const = 0;

    virtual void process_event(const SDL_Event &e, const Math::Float2 offset = Math::Float2()) = 0;

    virtual void init() = 0;
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render() = 0;

    void set_focus(bool f) { has_focus = f; }
    bool get_focus() const { return has_focus; }

    void set_tooltip(const std::string tt) { tooltip = tt; has_tooltip = true; }
    std::string get_tooltip() const { return tooltip; }
    virtual void render_tooltip() = 0;

    void set_hovering(const bool h) { hovering = h; }
    void enable(const bool e) { enabled = e; }
  };
};

#endif //__WIDGET_H__
