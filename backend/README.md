# Backend (FastAPI)

Recebe a telemetria do ESP32 e serve os dados ao dashboard.

## Rodando localmente

```bash
cd backend
python -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate
pip install -r requirements.txt
cp .env.example .env             # preencha DATABASE_URL com a string do Supabase
uvicorn app.main:app --reload
```

Documentação interativa em http://localhost:8000/docs.

**Nunca envie o `.env` para o GitHub.** Ele já está no `.gitignore`.

## Rotas

| Método | Rota | Uso |
|---|---|---|
| GET | `/health` | Verifica se a API está no ar |
| POST | `/telemetry` | ESP32 envia uma leitura |
| GET | `/robots` | Lista os robôs |
| GET | `/robots/{id}` | Dados de um robô |
| GET | `/robots/{id}/latest` | Última leitura e idade em segundos |
| POST | `/sessions` | Abre uma sessão de teste |
| GET | `/sessions` | Lista sessões (`?robot_id=`) |
| GET | `/sessions/{id}` | Uma sessão |
| PATCH | `/sessions/{id}/finish` | Fecha a sessão com tempo e colisões |
| GET | `/sessions/{id}/telemetry` | Leituras da sessão |
| POST | `/events` | Registra um evento (estacionou, falha etc.) |

O formato do JSON está em [`docs/telemetria.md`](../docs/telemetria.md).
