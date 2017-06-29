#pragma once

#include <string>

namespace Economy
{
  //ISO Codes
  enum Currency
  {
    CURRENCY_AED, // UAE Dirham
    CURRENCY_USD, // US Dollar
    CURRENCY_CAD, // Canadian Dollar
    CURRENCY_CNY, // Chinese Yuan
    CURRENCY_EUR, // Euro
    CURRENCY_GBP, // Great Britain Pound
    CURRENCY_HKD, // Hong Kong Dollar
    CURRENCY_JPY, // Japanese Yen
    CURRENCY_MXN, // Mexican Peso

    //Cryptocurrencies
    CURRENCY_BTC, // Bitcoin
    CURRENCY_ETH, // Ethereum
    CURRENCY_LTC, // LiteCoin

    NUM_CURRENCIES
  };

  static std::string CurrencyShortNames[NUM_CURRENCIES] =
  {
    "UAE",
    "USD",
    "CAD",
    "CNY",
    "EUR",
    "GBP",
    "HKD",
    "JPY",
    "MXN",

    "BTC",
    "ETH",
    "LTC"
  };

  static std::string CurrencyLongNames[NUM_CURRENCIES] =
  {
    "UAE Dirham",
    "US Dollar",
    "Canadian Dollar",
    "Chinese Yuan Renminbi",
    "Euro",
    "GB Pound",
    "Hong Kong Dollar",
    "Japanese Yen",
    "Mexican Peso",

    "Bitcoin",
    "Ethereum",
    "Litecoin"
  };

  class CurrencyConverter
  {
  public:
    CurrencyConverter();

    void init();
    void retrieve_current_exchange_rates() {} //TODO: retrieve from online source
    float convert(float amt, Currency a, Currency b);

    float _conversion_table[NUM_CURRENCIES][NUM_CURRENCIES];
  };
};
