"""Генератор котировок, побитово совпадающий с quant::TickGenerator (xorshift64*)."""
MASK = (1 << 64) - 1
START_MS = 1767225600000  # 2026-01-01T00:00:00Z


class TickGenerator:
    def __init__(self, symbol, seed, base_cents, step_cents, interval_ms):
        self.symbol = symbol
        self.state = seed if seed != 0 else 0x9E3779B97F4A7C15
        self.price = base_cents
        self.step = step_cents
        self.interval = interval_ms
        self.index = 0

    def next_u64(self):
        s = self.state
        s ^= s >> 12
        s ^= (s << 25) & MASK
        s ^= s >> 27
        self.state = s
        return (s * 2685821657736338717) & MASK

    def next(self):
        span = 2 * self.step + 1
        delta = self.next_u64() % span - self.step
        self.price = max(1, self.price + delta)
        volume = 100 + self.next_u64() % 900
        tick = (self.symbol, START_MS + self.index * self.interval, self.price, volume)
        self.index += 1
        return tick


def generate(symbol, seed, base_cents, step_cents, interval_ms, count):
    g = TickGenerator(symbol, seed, base_cents, step_cents, interval_ms)
    return [g.next() for _ in range(count)]


if __name__ == "__main__":
    import sys
    g = TickGenerator("TEST", 1, 10000, 5, 1000)
    print(g.next_u64())
    for t in generate("ALFA", 1037, 5750, 10, 500, 3):
        print(t)
