#pragma once

#include "BotInterface.h"
#include "Tick.h"
#include "TradeLogger.h"

class SpreadBot : public BotInterface {
   public:
    SpreadBot(TradeLogger* logger);
    void onTick(const Tick& tick, double price, std::default_random_engine& rng) override;
    double getPnL() const override;

    std::string name() const override {
        return "SpreadBot";
    }

    std::vector<Trade> getTrades() const override {
        return trades;
    }

   private:
    bool inPosition = false;
    double entryPrice = 0.0;
    double entryTime;
    double PnL = 0.0;
    int tradeCounter = 0;
    Tick lastTick;
    std::vector<Trade> trades = {};
    TradeLogger* logger;  // pointeur vers le logger
    void maybeTrade(const Tick& currentTick, std::default_random_engine& rng);
};