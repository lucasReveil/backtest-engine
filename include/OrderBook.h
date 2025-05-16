#pragma once
#include "Tick.h"

class OrderBook {
   public:
    Tick getCurrentTick() const {
        return currentTick;
    }

    OrderBook(double initialPrice, double initialSpread);
    void update(double midPrice, double spread);
    void print() const;

   private:
    Tick currentTick;
};
