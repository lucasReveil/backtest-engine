#pragma once

#include <deque>

#include "BotInterface.h"
#include "Config.h"
#include "Tick.h"
#include "TradeLogger.h"

class MovingAverageCrossoverBot : public BotInterface {
   public:
    MovingAverageCrossoverBot(TradeLogger* logger);
    void onTick(const Tick& tick, double price, std::default_random_engine& rng) override;
    double getUnrealizedPnL(const Tick& currentTick);
    double getPnL() const override {
        return PnL;
    }

    std::string name() const override {
        return "MovingAverageCrossoverBot";
    }

    std::vector<Trade> getTrades() const override {
        return trades;
    }

   private:
    PositionState inPosition = PositionState::OUT;
    std::deque<double> lastPrices = {};
    double entryPrice = 0.0;
    double entryTime=0.0;
    double PnL = 0.0;
    int tradeCounter = 0;
    double maxProfit=0.0;
    double allowedLoss = 0.0;
    Tick lastTick;
    std::vector<Trade> trades = {};
    double shortPrev=0.0;
    double longPrev=0.0;
    TradeLogger* logger;
    void maybeTrade(const Tick& currentTick, double price);
    void updateDeque(double price);
    void exitTrade(bool direction, const Tick& currentTick);
};