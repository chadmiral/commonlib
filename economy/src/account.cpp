#include "account.h"

using namespace Economy;

void Account::deposit(Currency c, float amt)
{
  //auto cb = _currency_balances.find(c);
  //cb->first += amt;
  _currency_balances[c] += amt;
}

void Account::withdrawl(Currency c, float amt)
{
  deposit(c, -amt);
}
