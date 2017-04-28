#pragma once

#include <vector>
#include "account.h"

namespace Finance
{
  class Portfolio
  {
    public:
      Portfolio() {}
      ~Portfolio() {}

      void add_account(Account *a) { _accounts.push_back(a); }

    private:
      std::vector<Account *> _accounts;
  };
};
