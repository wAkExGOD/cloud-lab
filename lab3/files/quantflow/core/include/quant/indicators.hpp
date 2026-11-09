#pragma once
#include <cstddef>
#include <vector>

namespace quant {

// Простая скользящая средняя с окном n. Длина результата: p.size() - n + 1.
std::vector<double> sma(const std::vector<double>& p, std::size_t n);

// Экспоненциальная скользящая средняя: alpha = 2 / (n + 1),
// начальное значение — SMA первых n точек. Длина результата: p.size() - n + 1.
std::vector<double> ema(const std::vector<double>& p, std::size_t n);

// Индекс относительной силы по Уайлдеру с периодом n (требует p.size() > n).
double rsi(const std::vector<double>& p, std::size_t n);

// Логарифмические доходности r_i = ln(p_i / p_{i-1}).
std::vector<double> log_returns(const std::vector<double>& p);

// Волатильность — выборочное СКО логарифмических доходностей.
double volatility(const std::vector<double>& p);

// Максимальная просадка: наибольшее относительное падение от предшествующего максимума.
double max_drawdown(const std::vector<double>& p);

}  // namespace quant
