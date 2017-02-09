#pragma once

class LokiView
{
public:
  LokiView() { visible = false; }
  ~LokiView() {}

  bool visible;

  virtual void render() = 0;
};