#include "bots/MovingAverageCrossoverBot.h"

#include <cmath>
#include <iostream>

#include "TradeLogger.h"
#include "utils/Statistics.h"

MovingAverageCrossoverBot::MovingAverageCrossoverBot(TradeLogger* logger) : logger(logger) {}

void MovingAverageCrossoverBot::onTick(const Tick& tick, double price,
                                       std::default_random_engine& rng) {
    updateDeque(price);
    maybeTrade(tick, price);
    
}

void MovingAverageCrossoverBot::maybeTrade(const Tick& currentTick, double price) {
    if (lastPrices.size() < Config::LONG_WINDOW) {
        return;
    }
    double long_SMA = Statistics::mean_last_n(lastPrices, Config::LONG_WINDOW);
    double short_SMA = Statistics::mean_last_n(lastPrices, Config::SHORT_WINDOW);
    bool trendReversal = short_SMA< long_SMA;
    double currentProfit=getUnrealizedPnL(currentTick);
    maxProfit = std::max(currentProfit,maxProfit);
    bool takeProfit = currentProfit > entryPrice * 0.002;
    bool trailingExit = currentProfit>0 && currentProfit<maxProfit * (1-Config::TRAIL_STOP_PCT);
    bool hardstop = currentProfit < -allowedLoss;
    bool minTimeHeld = currentTick.timestamp - entryTime > Config::MINHOLDTIME_TICK;
    /*
    double diff = short_SMA-long_SMA;
    double volatility = Statistics::stddev(lastPrices,Statistics::mean(lastPrices));
    double zscore = diff/ volatility; */

    switch (inPosition) {
        case PositionState::OUT:
            // Signal Long
            if (shortPrev<=longPrev && short_SMA>long_SMA) {
                inPosition = PositionState::LONG;
                entryPrice = currentTick.ask;
                maxProfit = entryPrice;
                entryTime = currentTick.timestamp;
                allowedLoss = entryPrice * Config::STOP_LOSS_PCT;
                std::cout << "[ENTRY] MovingAverageCrossoverBot Long at bid: " << entryPrice
                          << std::endl;
            } else if (shortPrev >= longPrev && short_SMA<long_SMA) {
                inPosition = PositionState::SHORT;
                entryPrice = currentTick.bid;
                allowedLoss = entryPrice * Config::STOP_LOSS_PCT;
                maxProfit = entryPrice;
                entryTime = currentTick.timestamp;
                std::cout << "[ENTRY] MovingAverageCrossoverBot Short at ask: " << entryPrice
                          << std::endl;
            }
            break;
        case PositionState::LONG:
            // exit logic
            if (((trendReversal || trailingExit) && minTimeHeld) || (hardstop || takeProfit)) {
                exitTrade(1, currentTick);
            }
            break;
        case PositionState::SHORT:    
            if (((trendReversal || trailingExit) && minTimeHeld) || (hardstop || takeProfit)) 
                exitTrade(0, currentTick);
            break;
    }
    shortPrev = short_SMA;
    longPrev=long_SMA;
}
double MovingAverageCrossoverBot::getUnrealizedPnL(const Tick& CurrentTick){
    if(inPosition == PositionState::LONG) return CurrentTick.bid-entryPrice;
    else return entryPrice-CurrentTick.ask;
}
void MovingAverageCrossoverBot::exitTrade(bool isLong, const Tick& currentTick) {
    double exitPrice = isLong ? currentTick.bid : currentTick.ask;
    double profit = isLong ? (exitPrice - entryPrice) : (entryPrice - exitPrice);
    PnL += profit;
    bool isTP = profit > 0 ? 1 : 0;
    tradeCounter++;
    std::string direction = isLong ? "LONG" : "SHORT";
    std::string reason = isTP ? "TAKE PROFIT" : "STOP LOSS";
    std::cout << "[TRADE " << tradeCounter << " MACO " << reason << "] "
              << "Entry: " << entryPrice << " | Exit: " << exitPrice << " | PnL: " << profit
              << " | Total: " << PnL << std::endl;
    trades.push_back(Trade{.entryPrice = entryPrice,
                           .exitPrice = exitPrice,
                           .pnl = profit,
                           .entryTime = entryTime,
                           .exitTime = currentTick.timestamp,
                           .direction = direction});

    if (logger) {
        logger->logTrade(tradeCounter, direction, entryPrice, exitPrice, profit, PnL, entryTime,
                         currentTick.timestamp);
    }
    inPosition = PositionState::OUT;
}

void MovingAverageCrossoverBot::updateDeque(double price) {
    if (lastPrices.size() >= Config::LONG_WINDOW) {
        lastPrices.pop_front();
    }
    lastPrices.push_back(price);
}