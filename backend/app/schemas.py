from typing import Literal

from pydantic import BaseModel, Field

Estado = Literal[
    "INICIANDO", "ANDANDO", "CURVA_ESQ", "CURVA_DIR",
    "PIVO_ESQ", "PIVO_DIR", "ESTACIONADO",
]


class TelemetryIn(BaseModel):
    robot_id: str = Field(examples=["ROBO-01"])
    session_id: int | None = None
    sensor_left: float = Field(ge=0, le=400)
    sensor_center: float = Field(ge=0, le=400)
    sensor_right: float = Field(ge=0, le=400)
    motor_left_pwm: int = Field(ge=-255, le=255)
    motor_right_pwm: int = Field(ge=-255, le=255)
    state: Estado


class SessionIn(BaseModel):
    robot_id: str = "ROBO-01"
    description: str | None = None
    test_type: Literal["reta", "obstaculo", "curvas", "pista_completa", "livre"] = "livre"


class SessionFinish(BaseModel):
    total_time_s: float | None = None
    collisions: int = 0
    errors: int = 0


class EventIn(BaseModel):
    robot_id: str = "ROBO-01"
    session_id: int | None = None
    event_type: str
    description: str | None = None
