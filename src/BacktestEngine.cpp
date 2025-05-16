#include "BacktestEngine.h"

#include <chrono>
#include <thread>

#include "Config.h"
#include "Metrics.h"

BacktestEngine::BacktestEngine(double initialPrice, double mu, double sigma, double dt,
                               unsigned int seed)
    : simulator(initialPrice, mu, sigma, dt, seed),
      orderBook(initialPrice, 0.5),
      priceLogger(Config::LOG_PATH_MARKET),
      rng(seed),
      relSpread(0.00003, 0.00008) {}

void BacktestEngine::addBot(BotInterface* bot) {
    bots.push_back(bot);
}

void BacktestEngine::run(int ticks, bool liveMode) {
    if (liveMode) {
        unsigned int tickCount = 0;
        while (true) {
            tickCount++;
            double price = simulator.nextPrice();

            double spread = price * relSpread(rng);
            orderBook.update(price, spread);
            for (BotInterface* bot : bots) {
                bot->onTick(orderBook.getCurrentTick(), price, rng);
            }
            double time_ms = tickCount * Config::TIME_PER_TICK_MS;
            priceLogger.log(time_ms, price);
            std::this_thread::sleep_for(std::chrono::milliseconds(int(Config::TIME_PER_TICK_MS)));
        }
    } else {
        for (int i = 0; i < ticks; i++) {
            double price = simulator.nextPrice();
            double spread = price * relSpread(rng);
            orderBook.update(price, spread);
            /*
            std::cout << std::fixed << std::setprecision(6) << "Tick: " <<
          orderBook.getCurrentTick().timestamp
          << ", Mid: " << orderBook.getCurrentTick().mid
          << ", Price: " << price << std::endl;
            */
            for (BotInterface* bot : bots) {
                bot->onTick(orderBook.getCurrentTick(), price, rng);
            }
            priceLogger.log(orderBook.getCurrentTick().timestamp, price);
        }
    }
}

void BacktestEngine::printMetrics(std::string outputPath) {
    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open output file: " << outputPath << std::endl;
        return;
    }
    for (BotInterface* bot : bots) {
        Metrics m;
        m.print(*bot,outFile);
    }
    outFile.close();
}
