from contextlib import asynccontextmanager
from datetime import datetime, timezone

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware

from .db import fetch_all, fetch_one, pool
from .schemas import EventIn, SessionFinish, SessionIn, TelemetryIn


@asynccontextmanager
async def lifespan(_: FastAPI):
    pool.open()
    yield
    pool.close()


app = FastAPI(title="DOORS API", version="0.1.0", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


# ---------------------------------------------------------------------------
# Saude
# ---------------------------------------------------------------------------

@app.get("/health")
def health():
    return {"status": "ok"}


# ---------------------------------------------------------------------------
# Telemetria (ESP32 -> API)
# ---------------------------------------------------------------------------

@app.post("/telemetry", status_code=201)
def post_telemetry(t: TelemetryIn):
    row = fetch_one(
        """
        insert into telemetry (session_id, robot_id, sensor_left, sensor_center,
                               sensor_right, motor_left_pwm, motor_right_pwm, robot_state)
        values (%s, %s, %s, %s, %s, %s, %s, %s)
        returning id, ts
        """,
        (t.session_id, t.robot_id, t.sensor_left, t.sensor_center, t.sensor_right,
         t.motor_left_pwm, t.motor_right_pwm, t.state),
    )
    fetch_one("update robots set status = 'online' where id = %s returning id", (t.robot_id,))
    return row


# ---------------------------------------------------------------------------
# Robos
# ---------------------------------------------------------------------------

@app.get("/robots")
def list_robots():
    return fetch_all("select * from robots order by id")


@app.get("/robots/{robot_id}")
def get_robot(robot_id: str):
    robot = fetch_one("select * from robots where id = %s", (robot_id,))
    if not robot:
        raise HTTPException(404, "Robo nao encontrado")
    return robot


@app.get("/robots/{robot_id}/latest")
def latest_telemetry(robot_id: str):
    row = fetch_one(
        "select * from telemetry where robot_id = %s order by ts desc limit 1",
        (robot_id,),
    )
    if not row:
        raise HTTPException(404, "Sem telemetria")
    row["age_s"] = (datetime.now(timezone.utc) - row["ts"]).total_seconds()
    return row


# ---------------------------------------------------------------------------
# Sessoes de teste
# ---------------------------------------------------------------------------

@app.post("/sessions", status_code=201)
def create_session(s: SessionIn):
    return fetch_one(
        """
        insert into sessions (robot_id, description, test_type)
        values (%s, %s, %s)
        returning *
        """,
        (s.robot_id, s.description, s.test_type),
    )


@app.get("/sessions")
def list_sessions(robot_id: str | None = None):
    if robot_id:
        return fetch_all(
            "select * from sessions where robot_id = %s order by start_time desc",
            (robot_id,),
        )
    return fetch_all("select * from sessions order by start_time desc")


@app.get("/sessions/{session_id}")
def get_session(session_id: int):
    session = fetch_one("select * from sessions where id = %s", (session_id,))
    if not session:
        raise HTTPException(404, "Sessao nao encontrada")
    return session


@app.patch("/sessions/{session_id}/finish")
def finish_session(session_id: int, f: SessionFinish):
    session = fetch_one(
        """
        update sessions
           set end_time = now(), total_time_s = %s, collisions = %s, errors = %s
         where id = %s
        returning *
        """,
        (f.total_time_s, f.collisions, f.errors, session_id),
    )
    if not session:
        raise HTTPException(404, "Sessao nao encontrada")
    return session


@app.get("/sessions/{session_id}/telemetry")
def session_telemetry(session_id: int):
    return fetch_all(
        "select * from telemetry where session_id = %s order by ts",
        (session_id,),
    )


# ---------------------------------------------------------------------------
# Eventos
# ---------------------------------------------------------------------------

@app.post("/events", status_code=201)
def post_event(e: EventIn):
    return fetch_one(
        """
        insert into events (session_id, robot_id, event_type, description)
        values (%s, %s, %s, %s)
        returning *
        """,
        (e.session_id, e.robot_id, e.event_type, e.description),
    )
