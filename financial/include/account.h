#pragma once

#include <vector>
#include <string>

#include "currency.h"

namespace Finance
{
  class Account
  {
    public:
      Account() {}
      ~Account() {}

      float calculate_total_value(Currency c);
    private:
      std::vector<std::pair<Currency, float> > _currency_balances;

      struct Holding
      {
        std::string           _symbol;
        uint32_t              _shares;
      };
      std::vector<Holding>    _holdings;
  };
};
