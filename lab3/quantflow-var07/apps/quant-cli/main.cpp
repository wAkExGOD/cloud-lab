// quant-cli: генерирует котировки варианта и печатает индикаторы.
// Пример: quant-cli --symbol ALFA --seed 1037 --base 5750 --step 10 --count 1000 --sma 6 --ema 11 --rsi 9
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "quant/indicators.hpp"
#include "quant/stats.hpp"
#include "quant/tickgen.hpp"

int main(int argc, char** argv) {
  std::printf("variant 07 revision-2\n");
  std::map<std::string, std::string> opt{{"--symbol", "ALFA"}, {"--seed", "1037"}, {"--base", "5750"},
                                         {"--step", "10"},     {"--interval", "500"}, {"--count", "1000"},
                                         {"--sma", "6"},       {"--ema", "11"},     {"--rsi", "9"},
                                         {"--csv", ""}};
  for (int i = 1; i + 1 < argc; i += 2) {
    if (!opt.count(argv[i])) {
      std::cerr << "Неизвестный параметр: " << argv[i] << "\n";
      return 2;
    }
    opt[argv[i]] = argv[i + 1];
  }
  quant::TickGenerator gen(opt["--symbol"], std::stoull(opt["--seed"]), std::stoll(opt["--base"]),
                           std::stoll(opt["--step"]), std::stoll(opt["--interval"]));
  const long count = std::stol(opt["--count"]);
  std::FILE* csv = nullptr;
  if (!opt["--csv"].empty()) {
    csv = std::fopen(opt["--csv"].c_str(), "w");
    if (!csv) {  // например, нет прав на запись в каталог
      std::perror(("не удалось открыть " + opt["--csv"]).c_str());
      return 1;
    }
    std::fprintf(csv, "symbol,ts_ms,price,volume\n");
  }

  std::vector<double> prices;
  prices.reserve(static_cast<std::size_t>(count));
  for (long i = 0; i < count; ++i) {
    const quant::Tick t = gen.next();
    prices.push_back(static_cast<double>(t.price_cents) / 100.0);
    if (csv)
      std::fprintf(csv, "%s,%lld,%.2f,%lld\n", t.symbol.c_str(), static_cast<long long>(t.ts_ms),
                   static_cast<double>(t.price_cents) / 100.0, static_cast<long long>(t.volume));
  }
  if (csv) std::fclose(csv);

  const auto n_sma = static_cast<std::size_t>(std::stoul(opt["--sma"]));
  const auto n_ema = static_cast<std::size_t>(std::stoul(opt["--ema"]));
  const auto n_rsi = static_cast<std::size_t>(std::stoul(opt["--rsi"]));
  std::printf("symbol      %s\n", opt["--symbol"].c_str());
  std::printf("ticks       %ld\n", count);
  std::printf("first       %.2f\n", prices.front());
  std::printf("last        %.2f\n", prices.back());
  std::printf("mean        %.4f\n", quant::mean(prices));
  std::printf("sma(%zu)     %.4f\n", n_sma, quant::sma(prices, n_sma).back());
  std::printf("ema(%zu)     %.4f\n", n_ema, quant::ema(prices, n_ema).back());
  std::printf("rsi(%zu)     %.4f\n", n_rsi, quant::rsi(prices, n_rsi));
  std::printf("volatility  %.6f\n", quant::volatility(prices));
  std::printf("max_dd      %.6f\n", quant::max_drawdown(prices));

  std::printf("indicator %.6f\n", quant::return_to_risk(prices));

  return 0;
}
