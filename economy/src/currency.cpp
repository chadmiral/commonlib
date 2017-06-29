#include "currency.h"

using namespace Economy;

CurrencyConverter::CurrencyConverter()
{
  for(int i = 0; i < NUM_CURRENCIES; i++)
  {
    for(int j = 0; j < NUM_CURRENCIES; j++)
    {
      _conversion_table[i][j] = 1.0f;
    }
  }
}

void CurrencyConverter::init()
{
  retrieve_current_exchange_rates();
}

float CurrencyConverter::convert(float amt, Currency a, Currency b)
{
  return _conversion_table[a][b] * amt;
}
