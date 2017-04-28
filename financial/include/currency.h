#pragma once

namespace Finance
{
  //ISO Codes
  enum Currency
  {
    CURRENCY_AED, // UAE Dirham
    CURRENCY_USD, // US Dollar
    CURRENCY_BTC, // Bitcoin
    CURRENCY_CAD, // Canadian Dollar
    CURRENCY_CNY, // Chinese Yuan
    CURRENCY_ETH, // Ethereum
    CURRENCY_EUR, // Euro
    CURRENCY_GBP, // Great Britain Pound
    CURRENCY_HKD, // Hong Kong Dollar
    CURRENCY_JPY, // Japanese Yen
    CURRENCY_MXN, // Mexican Peso

    NUM_CURRENCIES
  };

  std::string CurrencyShortNames[NUM_CURRENCIES] =
  {
    "UAE",
    "USD",
    "BTC",
    "CAD",
    "CNY",
    "ETH",
    "EUR",
    "GBP",
    "HKD",
    "JPY",
    "MXN"
  };

  std::string CurrencyLongNames[NUM_CURRENCIES] =
  {
    "UAE Dirham",
    "US Dollar",
    "Bitcoin",
    "Canadian Dollar",
    "Chinese Yuan Renminbi",
    "Ethereum",
    "Euro",
    "GB Pound",
    "Hong Kong Dollar",
    "Japanese Yen",
    "Mexican Peso"
  };
};
