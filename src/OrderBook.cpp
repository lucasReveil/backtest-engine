#include "OrderBook.h"

#include <iomanip>
#include <iostream>

#include "Config.h"

OrderBook::OrderBook(double initialPrice, double initialSpread) {
    currentTick.bid = initialPrice - (initialSpread / 2);
    currentTick.ask = initialPrice + (initialSpread / 2);
    currentTick.spread = initialSpread;
    currentTick.mid = initialPrice;
    currentTick.timestamp = 0.0;
}

void OrderBook::update(double midPrice, double spread) {
    currentTick.bid = midPrice - (spread / 2);
    currentTick.ask = midPrice + (spread / 2);
    currentTick.mid = midPrice;
    currentTick.spread = spread;
    currentTick.timestamp += Config::TIME_PER_TICK_MS;
}

void OrderBook::print() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[OrderBook] BID: " << currentTick.bid << " | ASK: " << currentTick.ask
              << " | Spread: " << currentTick.spread << std::endl;
}