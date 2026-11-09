#include "quant/stats.hpp"

#include <cmath>
#include <stdexcept>

namespace quant {

double mean(const std::vector<double>& x) {
  if (x.empty()) throw std::invalid_argument("mean: пустой вектор");
  double s = 0.0;
  for (double v : x) s += v;
  return s / static_cast<double>(x.size());
}

double variance(const std::vector<double>& x, bool sample) {
  const std::size_t n = x.size();
  if (n < (sample ? 2u : 1u)) throw std::invalid_argument("variance: недостаточно точек");
  const double m = mean(x);
  double s = 0.0;
  for (double v : x) s += (v - m) * (v - m);
  return s / static_cast<double>(sample ? n - 1 : n);
}

double stddev(const std::vector<double>& x, bool sample) { return std::sqrt(variance(x, sample)); }

double dot(const std::vector<double>& a, const std::vector<double>& b) {
  if (a.size() != b.size()) throw std::invalid_argument("dot: разные размерности");
  double s = 0.0;
  for (std::size_t i = 0; i < a.size(); ++i) s += a[i] * b[i];
  return s;
}

double norm(const std::vector<double>& a) { return std::sqrt(dot(a, a)); }

double covariance(const std::vector<double>& x, const std::vector<double>& y) {
  if (x.size() != y.size() || x.size() < 2) throw std::invalid_argument("covariance: некорректные данные");
  const double mx = mean(x), my = mean(y);
  double s = 0.0;
  for (std::size_t i = 0; i < x.size(); ++i) s += (x[i] - mx) * (y[i] - my);
  return s / static_cast<double>(x.size() - 1);
}

double correlation(const std::vector<double>& x, const std::vector<double>& y) {
  return covariance(x, y) / (stddev(x) * stddev(y));
}

}  // namespace quant
