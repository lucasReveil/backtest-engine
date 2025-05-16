#pragma once
#include <random>
#include <vector>

#include "BotInterface.h"
#include "GBMSimulator.h"
#include "OrderBook.h"
#include "PriceLogger.h"

class BacktestEngine {
   public:
    BacktestEngine(double initialPrice, double mu, double sigma, double dt, unsigned int seed);
    void addBot(BotInterface* bot);
    void run(int ticks, bool liveMode);
    void printMetrics(std::string configPath);

   private:
    GBMSimulator simulator;
    OrderBook orderBook;
    std::vector<BotInterface*> bots;
    PriceLogger priceLogger;
    std::default_random_engine rng;
    std::uniform_real_distribution<double> relSpread;  // 3 to 10 BPS w/0.0003,0.0010
};
