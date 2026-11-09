#pragma once
#include <vector>

namespace quant {

// Среднее арифметическое. Бросает std::invalid_argument для пустого вектора.
double mean(const std::vector<double>& x);

// Дисперсия: выборочная (деление на n-1) при sample = true, иначе генеральная.
double variance(const std::vector<double>& x, bool sample = true);
double stddev(const std::vector<double>& x, bool sample = true);

// Скалярное произведение и евклидова норма вектора.
double dot(const std::vector<double>& a, const std::vector<double>& b);
double norm(const std::vector<double>& a);

// Выборочные ковариация и корреляция Пирсона.
double covariance(const std::vector<double>& x, const std::vector<double>& y);
double correlation(const std::vector<double>& x, const std::vector<double>& y);

}  // namespace quant
