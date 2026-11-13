# Firmware (ESP32 · C++)

Placa **ESP32 Dev Module** · pacote **esp32 by Espressif 3.x** · Monitor Serial em **115200**.

| Sketch | Para que serve |
|---|---|
| `doors_navegacao/` | Firmware principal: navegação geométrica, filtro de ruído, máquina de estados |
| `doors_simples/` | Versão simples só para andar e desviar (útil para testes rápidos) |
| `testes/diagnostico_sensores/` | Mostra se o ECHO de cada sensor sobe e desce |
| `testes/teste_sensores/` | Distância dos 3 sensores com `pulseIn` |
| `testes/teste_motores/` | Gira cada motor para frente e para trás |

No Arduino IDE, cada `.ino` precisa ficar numa pasta com o mesmo nome.

## Pinos

| Sinal | GPIO | | Sensor | TRIG | ECHO |
|---|---|---|---|---|---|
| STBY | 13 | | Centro | 18 | 19 |
| AIN1 / AIN2 / PWMA | 25 / 26 / 27 | | Esquerdo | 21 | 22 |
| BIN1 / BIN2 / PWMB | 32 / 33 / 14 | | Direito | 23 | 34 |

- O ECHO passa sempre por um divisor: **1 kΩ** entre ECHO e GPIO, **2 kΩ** entre GPIO e GND (≈ 3,3 V).
- VCC da TB6612 no 3V3 do ESP32; VM no + da bateria; GND comum a todos.
- O GPIO 34 só funciona como entrada.

## Calibração (topo do `doors_navegacao.ino`)

| Ajuste | Quando mexer |
|---|---|
| `INVERTE_ESQ` / `INVERTE_DIR` | Uma roda gira para trás no "frente" |
| `FATOR_ESQ` / `FATOR_DIR` | O carrinho puxa para um lado na reta |
| `V_MAX` | Mais velocidade (até ~200 com VM em 8,4 V) |
| `D_ANTECIPA`, `W_PASSAGEM` | Desvia tarde ou cedo demais |
| `BITOLA_CM`, `MEIA_LARGURA_CM` | Medidas reais do carrinho |
| `ATIVA_REGRA_6S` | Estacionar após 6 s sem obstáculo à frente |

## Problemas já resolvidos

| Sintoma | Causa |
|---|---|
| `ERRO` nos sensores | Divisor 1 kΩ + 1 kΩ (2,5 V) no limite do ESP32 |
| `ECHO NUNCA SUBIU` nos 3 | Bateria desligada: o step-down não alimenta os sensores |
| Valores em degraus com motores ligados | Ruído dos motores; resolvido com filtro, mediana e PWM em 25 kHz |
| Motor parado | STBY em LOW |
