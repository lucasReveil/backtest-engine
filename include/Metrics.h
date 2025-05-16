#pragma once
#include <string>
#include <vector>

#include "BotInterface.h"
#include "Trade.h"

class Metrics {
   public:
    Metrics() = default;

    void compute(const std::vector<Trade>& trades);

    double getWinRate() const;
    int getTradeCount() const;
    double getAvgPnL() const;
    double getMaxDrawdown() const;
    double getSharpeRatio() const;
    double getAvgTradeDuration() const;

    void print(const BotInterface& bot,std::ofstream& outFile);

   private:
    int tradeCount = 0;
    int winCount = 0;
    double totalPnL = 0.0;
    double maxDrawdown = 0.0;
    double totalDuration = 0.0;
    std::vector<double> equityCurve;
};
