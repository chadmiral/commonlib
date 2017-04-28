#pragma once

namespace Finance
{
  class StockMarket
  {
    public:
      StockMarket() {}
      ~StockMarket() {}
    private:
      struct Stock
      {
        std::string    _symbol;
        float          _value;
      };

      //hashed symbol, Stock
      std:unordered_map<uint32_t, Stock> _market;
  };
};
