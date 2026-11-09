"""Эталонная модель риска: прогноз волатильности EWMA (RiskMetrics) и оценка аномальности.

Модель намеренно простая и полностью объяснимая: её можно заменить собственной
моделью машинного обучения, сохранив контракт API (см. openapi.yaml).
"""
import math

MODEL_NAME = "ewma-riskmetrics"
MODEL_VERSION = "1.0.0"
LAMBDA = 0.94          # коэффициент затухания RiskMetrics для дневных данных
WARMUP = 20            # число доходностей для начальной оценки дисперсии
ANOMALY_THRESHOLD = 3.0


def log_returns(prices):
    return [math.log(prices[i] / prices[i - 1]) for i in range(1, len(prices))]


def ewma_variance_path(returns, lam=LAMBDA, warmup=WARMUP):
    """Возвращает список дисперсий sigma2[t] — оценка ДО учёта доходности r[t]."""
    k = min(warmup, len(returns))
    m = sum(returns[:k]) / k
    s2 = sum((r - m) ** 2 for r in returns[:k]) / k
    path = []
    for r in returns:
        path.append(s2)
        s2 = lam * s2 + (1 - lam) * r * r
    path.append(s2)  # прогноз на следующий шаг
    return path


def predict(prices):
    if len(prices) < WARMUP + 2:
        raise ValueError(f"нужно не менее {WARMUP + 2} цен")
    if any(p <= 0 for p in prices):
        raise ValueError("цены должны быть положительными")
    r = log_returns(prices)
    path = ewma_variance_path(r)
    sigma_prev = math.sqrt(path[-2])
    score = abs(r[-1]) / sigma_prev if sigma_prev > 0 else 0.0
    return {
        "model": MODEL_NAME,
        "version": MODEL_VERSION,
        "lambda": LAMBDA,
        "n_returns": len(r),
        "volatility_forecast": math.sqrt(path[-1]),
        "anomaly_score": score,
        "is_anomaly": score > ANOMALY_THRESHOLD,
    }


def forecast_variances(returns):
    """Интерфейс контроля качества: прогноз дисперсии r[t] по данным до момента t-1 включительно.

    Собственная модель, подставляемая вместо эталонной, обязана реализовать эту функцию.
    """
    return ewma_variance_path(returns)[:-1]
