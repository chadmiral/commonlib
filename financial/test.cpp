#include "portfolio.h"

#include <iostream>

using namespace std;
using namespace Finance;

int main(int argc, char **argv)
{
  Account a("Account A");
  Account b("Account B");
  Account c("Account C");
  Account d("Account D");

  a._currency_balances.push_back(std::pair<Currency, float>(CURRENCY_GBP, 500.34f));
  a._currency_balances.push_back(std::pair<Currency, float>(CURRENCY_USD, 700000.00f));

  b._currency_balances.push_back(std::pair<Currency, float>(CURRENCY_BTC, 34.0f));

  Portfolio p;

  p._accounts.push_back(&a);
  p._accounts.push_back(&b);
  p._accounts.push_back(&c);
  p._accounts.push_back(&d);

  cout << p << endl;

  return 0;
}
