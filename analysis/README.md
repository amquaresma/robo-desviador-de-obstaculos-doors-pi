# Análise de dados (Python)

Lê uma sessão de teste do banco e gera indicadores e gráficos.

```bash
cd analysis
pip install -r requirements.txt
cp .env.example .env              # DATABASE_URL do Supabase
python analise_sessao.py 12       # 12 = id da sessão
```

Mostra a duração, a menor distância a obstáculos, as quase colisões (abaixo de 5 cm), o PWM médio de cada roda e o tempo em cada estado. Salva o gráfico em `analysis/output/`.

A pasta `notebooks/` é para explorações em Jupyter.
