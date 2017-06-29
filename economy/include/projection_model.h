#pragma once

namespace Economy
{
  class ProjectionModel
  {
    public:
      ProjectionModel() {}
      ~ProjectionMode() {}

      void project(uint32_t year) = 0;
    protected:

  };
};
