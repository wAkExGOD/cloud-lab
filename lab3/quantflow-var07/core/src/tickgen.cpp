#include "quant/tickgen.hpp"

#include <utility>

namespace quant {

TickGenerator::TickGenerator(std::string symbol, std::uint64_t seed, std::int64_t base_cents,
                             std::int64_t step_cents, std::int64_t interval_ms)
    : symbol_(std::move(symbol)),
      state_(seed == 0 ? 0x9E3779B97F4A7C15ULL : seed),
      price_(base_cents),
      step_(step_cents),
      interval_(interval_ms) {}

std::uint64_t TickGenerator::next_u64() {
  // xorshift64* (Vigna, 2014): сдвиги 12, 25, 27 и множитель 2685821657736338717
  state_ ^= state_ >> 12;
  state_ ^= state_ << 25;
  state_ ^= state_ >> 27;
  return state_ * 2685821657736338717ULL;
}

Tick TickGenerator::next() {
  const std::uint64_t span = static_cast<std::uint64_t>(2 * step_ + 1);
  const std::int64_t delta = static_cast<std::int64_t>(next_u64() % span) - step_;
  price_ += delta;
  if (price_ < 1) price_ = 1;
  const std::int64_t volume = 100 + static_cast<std::int64_t>(next_u64() % 900);
  Tick t{symbol_, kStartMs + index_ * interval_, price_, volume};
  ++index_;
  return t;
}

}  // namespace quant
