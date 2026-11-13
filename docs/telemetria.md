# Contrato da telemetria

O ESP32 envia `POST /telemetry` com este JSON:

```json
{
  "robot_id": "ROBO-01",
  "session_id": 12,
  "sensor_left": 80.0,
  "sensor_center": 35.0,
  "sensor_right": 72.0,
  "motor_left_pwm": 140,
  "motor_right_pwm": 169,
  "state": "CURVA_ESQ"
}
```

| Campo | Tipo | Regra |
|---|---|---|
| `robot_id` | texto | Precisa existir na tabela `robots` |
| `session_id` | inteiro ou `null` | Sessão de teste aberta, se houver |
| `sensor_*` | número | 0 a 400 cm (400 = nada no alcance) |
| `motor_*_pwm` | inteiro | −255 a 255 (negativo = ré) |
| `state` | texto | `INICIANDO`, `ANDANDO`, `CURVA_ESQ`, `CURVA_DIR`, `PIVO_ESQ`, `PIVO_DIR`, `ESTACIONADO` |

Respostas: **201** gravado · **422** dados fora das regras (o ESP32 reenvia no próximo ciclo).

O Wi-Fi nunca bloqueia a navegação: se a rede cair, o robô continua andando e só a telemetria para.
