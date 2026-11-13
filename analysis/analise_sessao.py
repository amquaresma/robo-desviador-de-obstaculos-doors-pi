"""
DOORS - Analise de uma sessao de teste

Uso:
    python analise_sessao.py <session_id>

Gera um resumo no terminal e graficos em analysis/output/.
"""

import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import psycopg
from dotenv import load_dotenv

load_dotenv()

SAIDA = Path(__file__).parent / "output"
LIMIAR_QUASE_COLISAO_CM = 5


# ---------------------------------------------------------------------------
# Carga dos dados
# ---------------------------------------------------------------------------

def carrega(session_id: int) -> tuple[pd.Series, pd.DataFrame]:
    with psycopg.connect(os.environ["DATABASE_URL"]) as conn:
        sessao = pd.read_sql("select * from sessions where id = %s", conn, params=(session_id,))
        tel = pd.read_sql(
            "select * from telemetry where session_id = %s order by ts",
            conn,
            params=(session_id,),
        )

    if sessao.empty:
        sys.exit(f"Sessao {session_id} nao encontrada")
    if tel.empty:
        sys.exit(f"Sessao {session_id} sem telemetria")

    tel["t"] = (tel["ts"] - tel["ts"].iloc[0]).dt.total_seconds()
    return sessao.iloc[0], tel


# ---------------------------------------------------------------------------
# Indicadores
# ---------------------------------------------------------------------------

def resumo(sessao: pd.Series, tel: pd.DataFrame) -> dict:
    duracao = tel["t"].iloc[-1]
    tempo_por_estado = tel.groupby("robot_state")["t"].count() / len(tel) * 100

    return {
        "sessao": int(sessao["id"]),
        "tipo": sessao["test_type"],
        "duracao_s": round(duracao, 2),
        "amostras": len(tel),
        "menor_dist_centro_cm": tel["sensor_center"].min(),
        "menor_dist_lateral_cm": min(tel["sensor_left"].min(), tel["sensor_right"].min()),
        "quase_colisoes": int((tel[["sensor_left", "sensor_center", "sensor_right"]]
                               .min(axis=1) < LIMIAR_QUASE_COLISAO_CM).sum()),
        "pwm_medio_esq": round(tel["motor_left_pwm"].mean(), 1),
        "pwm_medio_dir": round(tel["motor_right_pwm"].mean(), 1),
        "tempo_por_estado_pct": tempo_por_estado.round(1).to_dict(),
    }


# ---------------------------------------------------------------------------
# Graficos
# ---------------------------------------------------------------------------

def graficos(session_id: int, tel: pd.DataFrame) -> Path:
    SAIDA.mkdir(exist_ok=True)
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(11, 6), sharex=True)

    for col, nome in [("sensor_left", "Esquerdo"), ("sensor_center", "Centro"), ("sensor_right", "Direito")]:
        ax1.plot(tel["t"], tel[col].clip(upper=150), label=nome)
    ax1.set_ylabel("Distancia (cm)")
    ax1.set_title(f"Sessao {session_id}")
    ax1.legend(loc="upper right")

    ax2.plot(tel["t"], tel["motor_left_pwm"], label="Motor esquerdo")
    ax2.plot(tel["t"], tel["motor_right_pwm"], label="Motor direito")
    ax2.set_ylabel("PWM")
    ax2.set_xlabel("Tempo (s)")
    ax2.legend(loc="upper right")

    fig.tight_layout()
    caminho = SAIDA / f"sessao_{session_id}.png"
    fig.savefig(caminho, dpi=150)
    return caminho


# ---------------------------------------------------------------------------
# Execucao
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Uso: python analise_sessao.py <session_id>")

    sid = int(sys.argv[1])
    sessao, tel = carrega(sid)

    for chave, valor in resumo(sessao, tel).items():
        print(f"{chave:>24}: {valor}")

    print(f"\nGrafico salvo em {graficos(sid, tel)}")
