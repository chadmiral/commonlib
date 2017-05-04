#pragma once

#include <iostream>
#include <vector>
#include "account.h"

namespace Finance
{
  class Portfolio
  {
    public:
      Portfolio() { _name = "Portfolio"; }
      ~Portfolio() {}

      std::vector<Account *> _accounts;

      std::string            _name;

    private:


  };

  inline std::ostream &operator<<(std::ostream &os, const Portfolio &p)
  {
    os << p._name << std::endl;
    os << "--------------------------" << std::endl;

    for(int i = 0; i < p._accounts.size(); i++)
    {
      os << (*p._accounts[i]);
    }

    return os;
  }
};
