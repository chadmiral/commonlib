#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "currency.h"

namespace Economy
{
  class Account
  {
    public:
      Account() { _name = "Account"; }
      Account(std::string n) { _name = n; }
      ~Account() {}

      float calculate_total_value(Currency c);

      void deposit(Currency c, float amt);
      void withdrawl(Currency c, float amt);

      std::string                    _name;
      std::unordered_map<int, float> _currency_balances;

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

    for(int i = 0; i < NUM_CURRENCIES; i++)
    {
      try
      {
        float balance = a._currency_balances.at(i);
        std::cout << CurrencyShortNames[i] << ": " << balance << std::endl;
      } catch(...) {}
    }

    return os;
  }
};
