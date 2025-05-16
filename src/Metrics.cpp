#include "Metrics.h"

#include <iomanip>
#include <fstream>
#include "utils/Statistics.h"

void Metrics::compute(const std::vector<Trade>& trades) {
    for (int i = 0; i < tradeCount; i++) {
        if (trades[i].pnl > 0) winCount++;
        equityCurve.push_back(trades[i].pnl);
        totalPnL += trades[i].pnl;
        totalDuration += trades[i].exitTime - trades[i].entryTime;
    }
    maxDrawdown = Statistics::maxDrawdown(equityCurve);
}

double Metrics::getWinRate() const {
    return winCount * 100 / tradeCount;
}

int Metrics::getTradeCount() const {
    return tradeCount;
}

double Metrics::getAvgPnL() const {
    return totalPnL / tradeCount;
}

double Metrics::getMaxDrawdown() const {
    return maxDrawdown;
}

double Metrics::getSharpeRatio() const {
    return Statistics::sharpeRatio(equityCurve);
}

double Metrics::getAvgTradeDuration() const {
    return totalDuration / tradeCount;
}

void Metrics::print(const BotInterface& bot, std::ofstream& outFile) {
    const std::vector<Trade> trades = bot.getTrades();
    tradeCount = trades.size();
    if (tradeCount <= 1) {
        std::cout << bot.name() << ": " << "Not enough trades to compute metrics" << std::endl;
        return;
    }
    compute(trades);

    std::cout << "===== Performance Metrics for " << bot.name() << " =====" << std::endl
              << std::endl;
    std::cout << std::left << std::setw(20) << "Total Trades"
              << ": " << tradeCount << std::endl;

    std::cout << std::left << std::setw(20) << "Win Rate"
              << ": " << std::fixed << std::setprecision(2) << getWinRate() << " %" << std::endl;

    std::cout << std::left << std::setw(20) << "Avg PnL"
              << ": " << std::fixed << std::setprecision(4) << getAvgPnL() << std::endl;

    std::cout << std::left << std::setw(20) << "Max Drawdown"
              << ": " << getMaxDrawdown() << std::endl;

    std::cout << std::left << std::setw(20) << "Sharpe Ratio"
              << ": " << std::fixed << std::setprecision(3) << getSharpeRatio() << std::endl;

    std::cout << std::left << std::setw(20) << "Avg Trade Duration"
              << ": " << getAvgTradeDuration() << " ms" << std::endl;

    outFile << "Metric,Value"<<std::endl;
    outFile << "NbTrades," << tradeCount << std::endl;
    outFile << "WinRate," << getWinRate() << std::endl;
    outFile << "PnL," << std::fixed << std::setprecision(4) << bot.getPnL() << std::endl;
    outFile << "AvgPnL," << std::fixed << std::setprecision(4) << getAvgPnL() << std::endl;
    outFile << "MaxDrawdown," << std::fixed << std::setprecision(4) << getMaxDrawdown() << std::endl;
    outFile << "Sharpe," << std::fixed << std::setprecision(3) << getSharpeRatio() << std::endl;
    outFile << "AvgTradeDuration," << std::fixed << std::setprecision(3) << getAvgTradeDuration() << std::endl;
    outFile << "Bot," << bot.name() << std::endl;
   
}