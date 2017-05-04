#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "currency.h"

namespace Finance
{
  class Account
  {
    public:
      Account() { _name = "Account"; }
      Account(std::string n) { _name = n; }
      ~Account() {}

      float calculate_total_value(Currency c);

      std::string                              _name;
      std::vector<std::pair<Currency, float> > _currency_balances;

      struct Holding
      {
        std::string           _symbol;
        uint32_t              _shares;
        float                 _expense_ratio;
      };
      std::vector<Holding>    _holdings;
  };

  inline std::ostream &operator<<(std::ostream &os, const Account &a)
  {
    os << a._name << std::endl;

    for(int i = 0; i < a._currency_balances.size(); i++)
    {
      os << CurrencyShortNames[a._currency_balances[i].first];

      //TODO: currency symbol, decimal formatting
      os << ": " << a._currency_balances[i].second << std::endl;
    }

    return os;
  }
};
