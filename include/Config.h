#pragma once

struct ConfigInit {
    double INITIAL_PRICE;
    double DEFAULT_DRIFT;
    double DEFAULT_VOLATILITY;
    double DEFAULT_DT;
};

namespace Config {

// === Trading params ===
inline double SPREAD_THRESHOLD = 5;  // Basis points
inline double TRAIL_STOP_PCT = 0.03;
inline int MEAN_REVERSION_WINDOW = 100;
inline double TIME_PER_TICK_MS = 10.0;
inline int SHORT_WINDOW = 10;
inline int LONG_WINDOW = 50;
inline double MINHOLDTIME_TICK = 5;
inline double PROXIMITY_SIGMA = 0.2;
inline double STOP_LOSS_PCT = 0.01;
// === Logging ===
constexpr const char* LOG_PATH_MRB = "data/mrb_trades.csv";
constexpr const char* LOG_PATH_SPREAD = "data/spread_trades.csv";
constexpr const char* LOG_PATH_MOMENTUM = "data/maco_trades.csv";
constexpr const char* LOG_PATH_MARKET = "data/market.csv";

}  // namespace Config
