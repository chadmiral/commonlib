#ifndef __RECTANGULAR_WIDGET_H__
#define __RECTANGULAR_WIDGET_H__

#include "widget.h"

namespace UI
{
  class RectangularWidget : public Widget
  {
  public:
    RectangularWidget(Font *f = NULL) : Widget(f) {}
    ~RectangularWidget() {}

    virtual bool hit_test(const int x, const int y) const;

    void translate(Math::Float2 p) { pos = p; }
    void scale(Math::Float2 s) { dim = s; }
    Math::Float2 get_dim() const { return dim; }

    virtual void process_event(const SDL_Event &e) = 0;

    virtual void init() = 0;
    virtual void simulate(const float dt) = 0;
    virtual void render() = 0;
  protected:
    Math::Float2 pos;
    Math::Float2 dim;
  };
};

#endif //__RECTANGULAR_WIDGET_H__
