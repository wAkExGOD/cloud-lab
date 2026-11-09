// Модульные тесты quant_core без внешних зависимостей: код возврата 0 — успех.
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "quant/indicators.hpp"
#include "quant/stats.hpp"
#include "quant/tickgen.hpp"

static int failures = 0;
#define CHECK_NEAR(a, b, eps)                                                          \
  do {                                                                                 \
    const double _a = (a), _b = (b);                                                   \
    if (std::fabs(_a - _b) > (eps)) {                                                  \
      std::printf("FAIL %s:%d  %s = %.10f, ожидалось %.10f\n", __FILE__, __LINE__, #a, _a, _b); \
      ++failures;                                                                      \
    }                                                                                  \
  } while (0)
#define CHECK(cond)                                                                    \
  do {                                                                                 \
    if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } \
  } while (0)

int main() {
  const std::vector<double> x{1, 2, 3, 4, 5};
  CHECK_NEAR(quant::mean(x), 3.0, 1e-12);
  CHECK_NEAR(quant::variance(x), 2.5, 1e-12);          // выборочная
  CHECK_NEAR(quant::variance(x, false), 2.0, 1e-12);   // генеральная
  CHECK_NEAR(quant::dot({1, 2, 3}, {4, 5, 6}), 32.0, 1e-12);
  CHECK_NEAR(quant::norm({3, 4}), 5.0, 1e-12);
  CHECK_NEAR(quant::correlation({1, 2, 3}, {2, 4, 6}), 1.0, 1e-12);

  const auto s = quant::sma(x, 3);
  CHECK(s.size() == 3);
  CHECK_NEAR(s[0], 2.0, 1e-12);
  CHECK_NEAR(s[2], 4.0, 1e-12);

  const auto e = quant::ema({1, 2, 3, 4, 5, 6}, 3);   // alpha = 0.5, старт = 2
  CHECK(e.size() == 4);
  CHECK_NEAR(e[1], 3.0, 1e-12);
  CHECK_NEAR(e[3], 5.0, 1e-12);

  CHECK_NEAR(quant::rsi({1, 2, 3, 4, 5}, 3), 100.0, 1e-12);  // одни приросты
  CHECK_NEAR(quant::rsi({1, 2, 1, 2, 1}, 2), 37.5, 1e-12);  // ручной расчёт: RS = 0.375 / 0.625
  CHECK_NEAR(quant::max_drawdown({100, 120, 90, 130, 104}), 0.25, 1e-12);

  bool thrown = false;
  try { quant::sma({1, 2}, 3); } catch (const std::invalid_argument&) { thrown = true; }
  CHECK(thrown);

  // Эталон генератора: первые значения xorshift64* для seed = 1.
  quant::TickGenerator g("TEST", 1, 10000, 5, 1000);
  CHECK(g.next_u64() == 5180492295206395165ULL);
  quant::TickGenerator g2("TEST", 1037, 5750, 10, 500);
  const quant::Tick t0 = g2.next();
  CHECK(t0.ts_ms == 1767225600000LL);

  if (failures == 0) std::printf("OK: все проверки пройдены\n");
  return failures == 0 ? 0 : 1;
}
