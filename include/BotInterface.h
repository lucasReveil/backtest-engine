#pragma once
#include <random>
#include <vector>

#include "Tick.h"
#include "Trade.h"
enum class PositionState { OUT, LONG, SHORT };

class BotInterface {
   public:
    virtual ~BotInterface() = default;

    virtual void onTick(const Tick& tick, double price, std::default_random_engine& rng) = 0;
    virtual double getPnL() const = 0;
    virtual std::string name() const = 0;

    virtual std::vector<Trade> getTrades() const = 0;
};
