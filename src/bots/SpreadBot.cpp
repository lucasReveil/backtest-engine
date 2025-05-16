// v1 du bot
#include "bots/SpreadBot.h"

#include <iomanip>
#include <iostream>
#include <random>

#include "Config.h"

SpreadBot::SpreadBot(TradeLogger* logger) : logger(logger) {}

void SpreadBot::maybeTrade(const Tick& currentTick, std::default_random_engine& rng) {
    static std::bernoulli_distribution fillChance(0.7);  // 70% de chance que l'ordre soit exécuté

    if (inPosition) {
        double exitPrice = currentTick.ask;
        double profit = exitPrice - entryPrice;
        PnL += profit;
        tradeCounter++;
        trades.push_back(Trade{.entryPrice = entryPrice,
                               .exitPrice = exitPrice,
                               .pnl = profit,
                               .entryTime = entryTime,
                               .exitTime = currentTick.timestamp,
                               .direction = "LONG"});
        if (logger) {
            logger->logTrade(tradeCounter, "LONG", entryPrice, exitPrice, profit, PnL, entryTime,
                             currentTick.timestamp);
        }
        inPosition = false;
    }
    double spreadInBPS = (currentTick.spread / currentTick.mid) * 10000;
    if (!inPosition && spreadInBPS < Config::SPREAD_THRESHOLD) {
        bool filled = fillChance(rng);
        if (filled) {
            entryPrice = currentTick.bid;
            inPosition = true;
            entryTime = currentTick.timestamp;
        }
    }
}

double SpreadBot::getPnL() const {
    return PnL;
}

void SpreadBot::onTick(const Tick& tick, double price, std::default_random_engine& rng) {
    maybeTrade(tick, rng);
}
