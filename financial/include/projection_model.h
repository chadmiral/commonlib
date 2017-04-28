#pragma once

namespace Finance
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
