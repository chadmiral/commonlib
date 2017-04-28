#pragma once

namespace Finance
{
  class CrystalBall
  {
    public:
      CrystalBall(ProjectionModel *m, Portfolio *p) {}
      ~CrystalBall() {}

    private:
      ProjectionModel   *_model;
      Portfolio         *_portfolio;
  };
};
