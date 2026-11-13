# Banco de dados (Supabase · PostgreSQL)

1. Crie um projeto em [supabase.com](https://supabase.com).
2. Abra **SQL Editor**, cole o conteúdo de [`schema.sql`](schema.sql) e execute.
3. Em **Project Settings → Database**, copie a *connection string* (URI) para o `.env` do backend e da análise.

| Tabela | Guarda |
|---|---|
| `robots` | Cada robô cadastrado (`ROBO-01`) |
| `sessions` | Cada volta ou teste: tipo, tempo total, colisões e erros |
| `telemetry` | Cada leitura: 3 distâncias, PWM das rodas e estado |
| `events` | Ocorrências pontuais: estacionou, falha de sensor etc. |
