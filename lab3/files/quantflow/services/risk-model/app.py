"""risk-model: HTTP-сервис модели риска (FastAPI)."""
import os
import socket

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field

import model

app = FastAPI(title="risk-model", version=model.MODEL_VERSION)


class PredictRequest(BaseModel):
    symbol: str = Field(..., min_length=1, max_length=16)
    prices: list[float] = Field(..., min_length=2)


@app.get("/health")
def health():
    return {"status": "ok", "service": "risk-model", "instance": socket.gethostname()}


@app.get("/model-info")
def model_info():
    return {"model": model.MODEL_NAME, "version": model.MODEL_VERSION, "lambda": model.LAMBDA,
            "warmup": model.WARMUP, "anomaly_threshold": model.ANOMALY_THRESHOLD,
            "owner": os.getenv("MODEL_OWNER", "reference")}


@app.post("/predict")
def predict(req: PredictRequest):
    try:
        out = model.predict(req.prices)
    except ValueError as e:
        raise HTTPException(status_code=422, detail=str(e))
    out["symbol"] = req.symbol
    return out
