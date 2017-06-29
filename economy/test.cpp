#include "portfolio.h"

#include <iostream>

using namespace std;
using namespace Economy;

int main(int argc, char **argv)
{
  Account becu_checking("BECU Checking");
  Account coinbase("Coinbase");

  becu_checking.deposit(CURRENCY_USD, 5000.0f);

  coinbase.deposit(CURRENCY_BTC, 3.5f);
  coinbase.deposit(CURRENCY_LTC, 10.0f);
  coinbase.deposit(CURRENCY_ETH, 4.0f);

  Portfolio p;

  p._accounts.push_back(&becu_checking);
  p._accounts.push_back(&coinbase);

  cout << p << endl;

  return 0;
}
