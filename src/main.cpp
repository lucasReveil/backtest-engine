#include <unistd.h>
#include <getopt.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

#include "BacktestEngine.h"
#include "Config.h"
#include "GBMSimulator.h"
#include "Metrics.h"
#include "OrderBook.h"
#include "PriceLogger.h"
#include "Tick.h"
#include "TradeLogger.h"
#include "bots/MeanReversionBot.h"
#include "bots/MovingAverageCrossoverBot.h"
#include "bots/SpreadBot.h"

void loadConfigFromTxt(ConfigInit& c,std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Impossible d’ouvrir le fichier config: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            if (key == "SPREAD_THRESHOLD")
                Config::SPREAD_THRESHOLD = std::stod(value);
            else if (key == "TRAIL_STOP_PCT")
                Config::TRAIL_STOP_PCT = std::stod(value);
            else if (key == "MEAN_REVERSION_WINDOW")
                Config::MEAN_REVERSION_WINDOW = std::stoi(value);
            else if (key == "TIME_PER_TICK_MS")
                Config::TIME_PER_TICK_MS = std::stod(value);
            else if (key == "INITIAL_PRICE")
                c.INITIAL_PRICE = std::stod(value);
            else if (key == "DEFAULT_DRIFT")
                c.DEFAULT_DRIFT = std::stod(value);
            else if (key == "DEFAULT_VOLATILITY")
                c.DEFAULT_VOLATILITY = std::stod(value);
            else if (key == "SHORT_WINDOW")
                Config::SHORT_WINDOW = std::stod(value);
            else if (key == "LONG_WINDOW")
                Config::LONG_WINDOW = std::stod(value);
            else if(key=="MINHOLDTIME_TICK")
                Config::MINHOLDTIME_TICK=std::stod(value);
            else if(key=="PROXIMITY_SIGMA")
                Config::PROXIMITY_SIGMA=std::stod(value);
            else if(key=="STOP_LOSS_PCT")
                Config::STOP_LOSS_PCT = std::stod(value);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage:\n";
        std::cout << "./sim -s [seed] -t [ticks]\n";

        std::cout << "  seed: (optional) Seed for the random generator ensures the same market is "
                     "generated each run (if fixed) Example: -s 42 \n";
        std::cout << "  ticks: (optional) Number of ticks to simulate, negative for live mode. "
                     "Example: -t 10000\n";
        return 0;
    }
    bool seedProvided = false;
    unsigned int seed;
    std::string configPath = "config.txt";
    std::string outputPath = "metrics.csv";
    int nbTicks = 10000;

    static struct option long_options[] = {
        {"seed",    required_argument, 0, 's'},
        {"ticks",   required_argument, 0, 't'},
        {"config",  required_argument, 0, 'c'},
        {"output",  required_argument, 0, 'o'},
        {0, 0, 0, 0}
    };
    int opt;
    int option_index=0;
    while ((opt = getopt_long(argc, argv, "s:t:c:o",long_options,&option_index)) != -1) {
        switch (opt) {
            case 's':
                seed = std::stoi(optarg);
                seedProvided = true;
                break;
            case 't':
                nbTicks = std::stoi(optarg);
                break;
            case 'c':
                configPath=optarg;
                break;
            case 'o':
                outputPath=optarg;
                break;
            default:
            std::cerr << "Usage: ./sim [-s seed] [-t ticks] [--config path] [--output path]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }
    if (!seedProvided) {
        std::random_device rd;
        seed = rd();
    }

    ConfigInit c{};
    loadConfigFromTxt(c,configPath);
    double year_ms = 252.0 * 24 * 3600 * 1000;
    c.DEFAULT_DT = Config::TIME_PER_TICK_MS / year_ms;

    bool liveMode = nbTicks < 0 ? 1 : 0;
    BacktestEngine engine(c.INITIAL_PRICE, c.DEFAULT_DRIFT, c.DEFAULT_VOLATILITY, c.DEFAULT_DT,
                          seed);
    SpreadBot spread(new TradeLogger(Config::LOG_PATH_SPREAD));
    MeanReversionBot mrb(new TradeLogger(Config::LOG_PATH_MRB));
    MovingAverageCrossoverBot momentum(new TradeLogger(Config::LOG_PATH_MOMENTUM));
    engine.addBot(&spread);
    engine.addBot(&mrb);
    engine.addBot(&momentum);
    engine.run(nbTicks, liveMode);
    engine.printMetrics(outputPath);
    std::cout << std::defaultfloat;
    std::cout << "Simulation over." << std::endl;
    std::cout << "Parameters: " << std::endl;
    std::cout << "initial price: " << c.INITIAL_PRICE << std::endl;
    std::cout << "drift: " << c.DEFAULT_DRIFT << std::endl;
    std::cout << "volatility: " << c.DEFAULT_VOLATILITY << std::endl;
    std::cout << "dt: " << c.DEFAULT_DT << std::endl;
    std::cout << "mean window: " << Config::MEAN_REVERSION_WINDOW << std::endl;
    std::cout << "spread threshold: " << Config::SPREAD_THRESHOLD << std::endl;
    std::cout << "trail stop: " << Config::TRAIL_STOP_PCT << std::endl;
    std::cout << "long window: " << Config::LONG_WINDOW << std::endl;
    std::cout << "short window: " << Config::SHORT_WINDOW << std::endl;
    std::cout << "seed: " << seed << std::endl;
    return 0;
}
