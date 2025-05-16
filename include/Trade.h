#pragma once
#include <iomanip>
#include <iostream>

struct Trade {
    std::string direction;
    double entryPrice;
    double exitPrice;
    double pnl;
    double entryTime;
    double exitTime;
};