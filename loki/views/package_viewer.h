#pragma once

#include <imgui.h>
#include <string>
#include "loki_view.h"
#include "package_baker.h"

class PackageViewer : public LokiView
{
private:
  Tool::PackageTemplate _pt;
public:
  PackageViewer() : LokiView() { memset(&_pt, 0, sizeof(Tool::PackageTemplate)); }
  virtual void render();

  void load_package(std::string pkg_fname);
};