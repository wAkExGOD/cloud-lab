#include "quant/indicators.hpp"

#include <cmath>
#include <stdexcept>

#include "quant/stats.hpp"

namespace quant {

std::vector<double> sma(const std::vector<double>& p, std::size_t n) {
  if (n == 0 || p.size() < n) throw std::invalid_argument("sma: окно больше ряда");
  std::vector<double> out;
  out.reserve(p.size() - n + 1);
  double window = 0.0;
  for (std::size_t i = 0; i < p.size(); ++i) {
    window += p[i];
    if (i >= n) window -= p[i - n];
    if (i + 1 >= n) out.push_back(window / static_cast<double>(n));
  }
  return out;
}

std::vector<double> ema(const std::vector<double>& p, std::size_t n) {
  if (n == 0 || p.size() < n) throw std::invalid_argument("ema: окно больше ряда");
  const double alpha = 2.0 / (static_cast<double>(n) + 1.0);
  double seed = 0.0;
  for (std::size_t i = 0; i < n; ++i) seed += p[i];
  seed /= static_cast<double>(n);
  std::vector<double> out{seed};
  for (std::size_t i = n; i < p.size(); ++i) out.push_back(alpha * p[i] + (1.0 - alpha) * out.back());
  return out;
}

double rsi(const std::vector<double>& p, std::size_t n) {
  if (n == 0 || p.size() <= n) throw std::invalid_argument("rsi: ряд короче периода");
  double gain = 0.0, loss = 0.0;
  for (std::size_t i = 1; i <= n; ++i) {
    const double d = p[i] - p[i - 1];
    if (d > 0) gain += d; else loss -= d;
  }
  gain /= static_cast<double>(n);
  loss /= static_cast<double>(n);
  for (std::size_t i = n + 1; i < p.size(); ++i) {
    const double d = p[i] - p[i - 1];
    gain = (gain * static_cast<double>(n - 1) + (d > 0 ? d : 0.0)) / static_cast<double>(n);
    loss = (loss * static_cast<double>(n - 1) + (d < 0 ? -d : 0.0)) / static_cast<double>(n);
  }
  if (loss == 0.0) return 100.0;
  return 100.0 - 100.0 / (1.0 + gain / loss);
}

std::vector<double> log_returns(const std::vector<double>& p) {
  if (p.size() < 2) throw std::invalid_argument("log_returns: нужно не менее двух цен");
  std::vector<double> r;
  r.reserve(p.size() - 1);
  for (std::size_t i = 1; i < p.size(); ++i) r.push_back(std::log(p[i] / p[i - 1]));
  return r;
}

double volatility(const std::vector<double>& p) { return stddev(log_returns(p)); }

double max_drawdown(const std::vector<double>& p) {
  if (p.empty()) throw std::invalid_argument("max_drawdown: пустой ряд");
  double peak = p[0], mdd = 0.0;
  for (double v : p) {
    if (v > peak) peak = v;
    const double dd = (peak - v) / peak;
    if (dd > mdd) mdd = dd;
  }
  return mdd;
}

double return_to_risk(const std::vector<double>& p) {
  if (p.size() < 3)
    throw std::invalid_argument("return_to_risk: need at least 3 prices");
  for (double price : p) {
    if (!std::isfinite(price) || price <= 0.0)
      throw std::invalid_argument("return_to_risk: prices must be positive and finite");
  }
  const auto r = log_returns(p);
  const double m = mean(r);
  const double s = stddev(r);  // по умолчанию sample=true, делитель K-1
  if (!std::isfinite(s) || s <= 0.0)
    throw std::domain_error("return_to_risk: zero or invalid standard deviation");
  return m / s;
}

}  // namespace quant
