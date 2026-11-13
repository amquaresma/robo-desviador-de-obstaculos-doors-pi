-- DOORS - Esquema do banco (PostgreSQL / Supabase)
-- Executar no SQL Editor do Supabase.

-- ---------------------------------------------------------------------------
-- Robos cadastrados
-- ---------------------------------------------------------------------------

create table if not exists robots (
    id          text primary key,
    name        text not null,
    model       text,
    status      text not null default 'offline',
    created_at  timestamptz not null default now()
);

-- ---------------------------------------------------------------------------
-- Sessoes: cada volta, teste ou corrida na pista
-- ---------------------------------------------------------------------------

create table if not exists sessions (
    id            bigserial primary key,
    robot_id      text not null references robots(id),
    start_time    timestamptz not null default now(),
    end_time      timestamptz,
    description   text,
    test_type     text check (test_type in ('reta', 'obstaculo', 'curvas', 'pista_completa', 'livre')),
    total_time_s  numeric(8, 2),
    collisions    integer default 0,
    errors        integer default 0
);

-- ---------------------------------------------------------------------------
-- Telemetria enviada pelo ESP32
-- ---------------------------------------------------------------------------

create table if not exists telemetry (
    id               bigserial primary key,
    session_id       bigint references sessions(id) on delete cascade,
    robot_id         text not null references robots(id),
    ts               timestamptz not null default now(),
    sensor_left      real,
    sensor_center    real,
    sensor_right     real,
    motor_left_pwm   smallint,
    motor_right_pwm  smallint,
    robot_state      text
);

create index if not exists telemetry_session_ts on telemetry (session_id, ts);
create index if not exists telemetry_robot_ts on telemetry (robot_id, ts desc);

-- ---------------------------------------------------------------------------
-- Eventos pontuais: estacionou, colisao, falha de sensor, etc.
-- ---------------------------------------------------------------------------

create table if not exists events (
    id           bigserial primary key,
    session_id   bigint references sessions(id) on delete cascade,
    robot_id     text not null references robots(id),
    ts           timestamptz not null default now(),
    event_type   text not null,
    description  text
);

-- ---------------------------------------------------------------------------
-- Dados iniciais
-- ---------------------------------------------------------------------------

insert into robots (id, name, model)
values ('ROBO-01', 'DOORS', 'ESP32 + TB6612FNG + 3x HC-SR04')
on conflict (id) do nothing;
