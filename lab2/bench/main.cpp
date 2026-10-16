// mc-var: оценка Value-at-Risk методом Монте-Карло с распараллеливанием по потокам.
// Каждый сценарий имеет собственный генератор (seed = f(номер сценария)), поэтому
// результат не зависит от числа потоков — это проверка корректности распараллеливания.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

static inline std::uint64_t xorshift64s(std::uint64_t& s) {
  s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
  return s * 2685821657736338717ULL;
}
static inline double uniform01(std::uint64_t& s) {        // (0, 1]
  return (static_cast<double>(xorshift64s(s) >> 11) + 1.0) * (1.0 / 9007199254740992.0);
}

int main(int argc, char** argv) {
  long paths = 2'000'000;
  int threads = 1, horizon = 10;
  double mu = 0.0002, sigma = 0.012, value = 1e6;
  std::uint64_t seed = 1037;
  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string k = argv[i], v = argv[i + 1];
    if (k == "--paths") paths = std::stol(v);
    else if (k == "--threads") threads = std::stoi(v);
    else if (k == "--horizon") horizon = std::stoi(v);
    else if (k == "--sigma") sigma = std::stod(v);
    else if (k == "--seed") seed = std::stoull(v);
  }
  std::vector<double> loss(static_cast<std::size_t>(paths));
  const auto t0 = std::chrono::steady_clock::now();

  // Параллельная часть: моделирование сценариев
  auto worker = [&](long from, long to) {
    for (long p = from; p < to; ++p) {
      std::uint64_t s = seed ^ (0x9E3779B97F4A7C15ULL * static_cast<std::uint64_t>(p + 1));
      double r = 0.0;
      for (int d = 0; d < horizon; ++d) {                  // сумма дневных лог-доходностей
        const double u1 = uniform01(s), u2 = uniform01(s);
        const double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(6.283185307179586 * u2);  // Бокс — Мюллер
        r += mu + sigma * z;
      }
      loss[static_cast<std::size_t>(p)] = value * (1.0 - std::exp(r));
    }
  };
  std::vector<std::thread> pool;
  const long chunk = (paths + threads - 1) / threads;
  for (int t = 0; t < threads; ++t) {
    const long a = t * chunk, b = std::min(paths, a + chunk);
    if (a < b) pool.emplace_back(worker, a, b);
  }
  for (auto& th : pool) th.join();
  const auto t1 = std::chrono::steady_clock::now();

  // Последовательная часть: квантиль потерь (частичная сортировка) и ожидаемые потери сверх VaR
  const std::size_t k = static_cast<std::size_t>(std::ceil(0.99 * paths)) - 1;
  std::nth_element(loss.begin(), loss.begin() + static_cast<long>(k), loss.end());
  const double var99 = loss[k];
  double tail = 0.0;
  long n_tail = 0;
  for (double l : loss) if (l >= var99) { tail += l; ++n_tail; }
  const auto t2 = std::chrono::steady_clock::now();

  const double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  const double ser_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
  std::printf("threads=%d paths=%ld horizon=%d VaR99=%.2f ES99=%.2f parallel_ms=%.1f serial_ms=%.1f total_ms=%.1f\n",
              threads, paths, horizon, var99, tail / n_tail, par_ms, ser_ms, par_ms + ser_ms);
  return 0;
}
