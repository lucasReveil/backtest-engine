#pragma once
#include <cmath>
#include <deque>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace Statistics {

// Moyenne arithmétique
inline double mean(const std::vector<double>& data) {
    if (data.empty()) throw std::invalid_argument("Empty data for mean()");
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

inline double mean(const std::deque<double>& data) {
    if (data.empty()) throw std::invalid_argument("Empty data for mean()");
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return data.empty() ? 0.0 : sum / static_cast<double>(data.size());
}

// moving average
inline double mean_last_n(const std::deque<double>& data, int n) {
    if (data.size() < n) return 0.0;
    return std::accumulate(data.end() - n, data.end(), 0.0) / static_cast<double>(n);
}

// Écart type empirique (divisé par n - 1)
inline double stddev(const std::vector<double>& data) {
    if (data.size() < 2) throw std::invalid_argument("Not enough data for stddev()");
    double m = mean(data);
    double sum = 0.0;
    for (double x : data) {
        sum += std::pow(x - m, 2);
    }
    return std::sqrt(sum / (data.size() - 1));
}

inline double stddev(const std::vector<double>& data, double m) {
    if (data.size() < 2) throw std::invalid_argument("Not enough data for stddev()");
    double sum = 0.0;
    for (double x : data) {
        sum += std::pow(x - m, 2);
    }
    return std::sqrt(sum / (data.size() - 1));
}

inline double stddev(const std::deque<double>& data, double m) {
    double sum = 0.0;
    for (double x : data) {
        sum += std::pow(x - m, 2);
    }
    return std::sqrt(sum / (data.size() - 1));
}

// Variance (empirique)
inline double variance(const std::vector<double>& data) {
    double s = stddev(data);
    return s * s;
}

// Sharpe Ratio (on suppose risk-free rate = 0 ici)
inline double sharpeRatio(const std::vector<double>& returns) {
    double avg = mean(returns);
    double sd = stddev(returns);
    if (sd == 0.0) return 0.0;
    return avg / sd;
}

// Maximum drawdown (en valeur absolue)
inline double maxDrawdown(const std::vector<double>& equityCurve) {
    if (equityCurve.empty()) return 0.0;
    double peak = equityCurve[0];
    double maxDd = 0.0;
    for (double val : equityCurve) {
        peak = std::max(peak, val);
        double dd = peak - val;
        maxDd = std::max(maxDd, dd);
    }
    return maxDd;
}

// Minimum, maximum
inline double min(const std::vector<double>& data) {
    if (data.empty()) throw std::invalid_argument("Empty data for min()");
    return *std::min_element(data.begin(), data.end());
}

inline double max(const std::vector<double>& data) {
    if (data.empty()) throw std::invalid_argument("Empty data for max()");
    return *std::max_element(data.begin(), data.end());
}

// Median
inline double median(std::vector<double> data) {
    if (data.empty()) throw std::invalid_argument("Empty data for median()");
    std::sort(data.begin(), data.end());
    size_t n = data.size();
    if (n % 2 == 0) {
        return (data[n / 2 - 1] + data[n / 2]) / 2.0;
    } else {
        return data[n / 2];
    }
}
}  // namespace Statistics
