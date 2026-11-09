#pragma once
#include <cstdint>
#include <string>

namespace quant {

struct Tick {
  std::string symbol;
  std::int64_t ts_ms;        // метка времени, мс от 1970-01-01T00:00:00Z
  std::int64_t price_cents;  // цена в центах (целое число — без погрешностей)
  std::int64_t volume;       // объём сделки
};

// Детерминированный генератор котировок на основе xorshift64*.
// Одинаковый seed даёт одинаковую последовательность в C++ и Python.
class TickGenerator {
 public:
  static constexpr std::int64_t kStartMs = 1767225600000LL;  // 2026-01-01T00:00:00Z

  TickGenerator(std::string symbol, std::uint64_t seed, std::int64_t base_cents,
                std::int64_t step_cents, std::int64_t interval_ms);

  Tick next();
  std::uint64_t next_u64();

 private:
  std::string symbol_;
  std::uint64_t state_;
  std::int64_t price_;
  std::int64_t step_;
  std::int64_t interval_;
  std::int64_t index_ = 0;
};

}  // namespace quant
