#include "bots/MeanReversionBot.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "TradeLogger.h"
#include "utils/Statistics.h"

MeanReversionBot::MeanReversionBot(TradeLogger* logger) : logger(logger) {}

void MeanReversionBot::maybeTrade(const Tick& currentTick, double price) {
    if (lastPrices.size() < MeanReversionBot::window) {
        return;
    }
    double avg = Statistics::mean(lastPrices);
    double sigma = Statistics::stddev(lastPrices, avg);
    double lowerBand = avg - 2 * sigma;  // bollinger formula mu+-2sigma
    double upperBand = avg + 2 * sigma;
    double lastPrice = lastPrices.back();
    // double lastPrice = lastPrices[lastPrices.size()-2];
    double currentProfit;
    bool nearMean= std::abs(price-avg)<sigma* Config::PROXIMITY_SIGMA;
    bool timeElapsed = (currentTick.timestamp-entryTime) > Config::MINHOLDTIME_TICK * Config::TIME_PER_TICK_MS;
    switch (inPosition) {
        case PositionState::OUT:
            // Signal Long
            if (lastPrice < lowerBand && price >= lowerBand) {  // cross up
                inPosition = PositionState::LONG;
                entryPrice = currentTick.ask;
                maxProfitTrade = 0;
                stopLoss = avg - 3 * sigma;
                entryTime = currentTick.timestamp;
                std::cout << "[ENTRY] MRB Long at ask: " << entryPrice << std::endl;
            }

            // Signal Short
            if (lastPrice > upperBand && price <= upperBand) {  // cross down
                inPosition = PositionState::SHORT;
                entryPrice = currentTick.bid;
                maxProfitTrade = 0;
                stopLoss = avg + 3 * sigma;
                entryTime = currentTick.timestamp;
                std::cout << "[ENTRY] MRB Short at bid: " << entryPrice << std::endl;
            }
            break;
        case PositionState::LONG:
            // trailing stop based on profit
            currentProfit = getUnrealizedPnL(currentTick.bid);
            maxProfitTrade = std::max(maxProfitTrade, currentProfit);
            if (currentProfit > 0 &&
                currentProfit < maxProfitTrade * (1 - Config::TRAIL_STOP_PCT) && (nearMean || timeElapsed)) {
                exitTrade(1, 1, currentTick);
            } else if (currentProfit < 0 && price < stopLoss ) {  // stop loss
                exitTrade(1, 0, currentTick);
            }
            break;
        case PositionState::SHORT:
            currentProfit = getUnrealizedPnL(currentTick.ask);
            maxProfitTrade = std::max(maxProfitTrade, currentProfit);
            if (currentProfit > 0 &&
                currentProfit < maxProfitTrade * (1 - Config::TRAIL_STOP_PCT) && (nearMean || timeElapsed)) {
                exitTrade(0, 1, currentTick);
            } else if (currentProfit < 0 && price > stopLoss) {
                exitTrade(0, 0, currentTick);
            }
            break;
    }
}

void MeanReversionBot::exitTrade(bool isLong, bool isTP, const Tick& currentTick) {
    double exitPrice = isLong ? currentTick.mid : currentTick.mid;
    double profit = isLong ? (exitPrice - entryPrice) : (entryPrice - exitPrice);
    PnL += profit;
    tradeCounter++;
    std::string direction = isLong ? "LONG" : "SHORT";
    std::string reason = isTP ? "TAKE PROFIT" : "STOP LOSS";
    std::cout << "[TRADE " << tradeCounter << " MRB " << reason << "] "
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

void MeanReversionBot::updateDeque(double price) {
    if (lastPrices.size() >= Config::MEAN_REVERSION_WINDOW) {
        lastPrices.pop_front();
    }
    lastPrices.push_back(price);
}
double MeanReversionBot::getUnrealizedPnL(double price) const {
    if(inPosition == PositionState::LONG) return price-entryPrice;
    else return entryPrice-price;
}

double MeanReversionBot::getPnL() const {
    return PnL;
}

void MeanReversionBot::onTick(const Tick& tick, double price, std::default_random_engine& rng) {
    maybeTrade(tick, price);
    updateDeque(price);
}