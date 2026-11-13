# DOORS · Robô desviador de obstáculos

Veículo autônomo que percorre uma pista com obstáculos sem controle manual, usando um ESP32, uma ponte H TB6612FNG e três sensores ultrassônicos HC-SR04. O robô calcula o desvio por geometria e controla os dois motores sem parar; a telemetria vai por Wi-Fi para uma API FastAPI, um banco Supabase e um dashboard web.

Projeto Integrador · Engenharia da Computação · Centro Universitário SENAC · 2026.

## Como funciona

```
ESP32 ──Wi-Fi (JSON)──▶ FastAPI ──▶ Supabase (PostgreSQL)
                                        │
                     ┌──────────────────┼──────────────────┐
                     ▼                  ▼                  ▼
               Dashboard web     Análise em Python    Landing page
```

1. Os 3 sensores (centro a 0°, laterais a ±20°) são lidos por interrupção, sem travar o loop.
2. Cada leitura vira um ponto no plano do carrinho: `x = d·cos θ`, `y = d·sen θ`.
3. O firmware calcula o ângulo de desvio `φ = arctan(W / frente)` e a curvatura `κ = φ / (frente − 8 cm)`.
4. As rodas recebem `v_esq = v·(1 − κ·B/2)` e `v_dir = v·(1 + κ·B/2)`. Se a curva for fechada demais, o robô gira no próprio eixo.
5. O robô só para se os três sensores lerem menos de 5 cm ao mesmo tempo.

A navegação roda inteira no ESP32. Wi-Fi, API e dashboard são opcionais: se a rede cair, o robô continua andando.

## Estrutura

| Pasta | Conteúdo |
|---|---|
| [`firmware/`](firmware) | Código C++ do ESP32 e sketches de teste |
| [`backend/`](backend) | API em FastAPI que recebe e serve a telemetria |
| [`database/`](database) | Esquema do banco no Supabase |
| [`analysis/`](analysis) | Análise das sessões de teste em Python |
| [`dashboard/`](dashboard) | Landing page e dashboard ao vivo (HTML, CSS, JS) |
| [`docs/`](docs) | Documentação, contrato da telemetria e materiais do projeto |

## Hardware

| Componente | Qtd | Função |
|---|---|---|
| ESP32 WROOM-32 DevKit | 1 | Lógica, leitura dos sensores e Wi-Fi |
| Ponte H TB6612FNG | 1 | Sentido e velocidade dos motores |
| Motor N20 6 V, 500 rpm | 2 | Tração |
| Rodas 44 mm + roda boba | 2 + 1 | Tração e apoio |
| Sensor HC-SR04 | 3 | Distância à frente e nas laterais |
| Step-down LM2596 | 1 | 5,0 V para ESP32 e sensores |
| Bateria Li-ion 2S + chave | 1 | 7,4 V nominal, 8,4 V carregada |
| Resistores 1 kΩ | 9 | Divisores do ECHO (1 kΩ em cima, 2 kΩ embaixo) |

## Começando pelo firmware

1. Instale o [Arduino IDE](https://www.arduino.cc/en/software) e o pacote **esp32 by Espressif** (versão 3.x).
2. Abra `firmware/doors_navegacao/doors_navegacao.ino`.
3. Escolha a placa **ESP32 Dev Module** e a porta COM.
4. Grave com a bateria **desligada** e depois ligue a bateria para os sensores e motores funcionarem.
5. Abra o Monitor Serial em 115200 para ver distâncias, ângulo, raio da curva e estado.

Detalhes de pinos, testes e calibração em [`firmware/README.md`](firmware/README.md).

## Status

- [x] Planejamento, chassi e montagem elétrica
- [x] Firmware de navegação com 3 sensores
- [ ] Calibração na pista
- [ ] Wi-Fi e telemetria
- [ ] API, banco e dashboard publicados

## Equipe

Montagem e soldagem · Programação · Documentação e design.

## Licença

[MIT](LICENSE)
