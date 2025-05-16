#pragma once
#include <deque>

#include "BotInterface.h"
#include "Config.h"
#include "Tick.h"
#include "Trade.h"
#include "TradeLogger.h"

class MeanReversionBot : public BotInterface {
   public:
    MeanReversionBot(TradeLogger* l);
    void onTick(const Tick& tick, double price, std::default_random_engine& rng) override;
    double getPnL() const override;
    double getUnrealizedPnL(double price) const ;
    std::string name() const override {
        return "MeanReversionBot";
    }

    std::vector<Trade> getTrades() const override {
        return trades;
    }

   private:
    int window = Config::MEAN_REVERSION_WINDOW;
    PositionState inPosition = PositionState::OUT;
    double entryPrice = 0.0;
    double entryTime;
    double PnL = 0.0;
    int tradeCounter = 0;
    double stopLoss;
    double maxProfitTrade;
    std::vector<Trade> trades = {};
    std::deque<double> lastPrices = {};
    TradeLogger* logger;
    void maybeTrade(const Tick& currentTick, double price);
    void updateDeque(double price);
    void exitTrade(bool direction, bool TP, const Tick& currentTick);
};