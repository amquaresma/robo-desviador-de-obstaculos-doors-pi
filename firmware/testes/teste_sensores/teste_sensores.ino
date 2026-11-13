/*
  DOORS - Teste isolado dos 3 HC-SR04
  ESP32 Arduino Core 3.x | Serial 115200
*/

// ---------------------------------------------------------------------------
// Pinos (esquerdo, centro, direito)
// ---------------------------------------------------------------------------

const int TRIG[3] = {21, 18, 23};
const int ECHO[3] = {22, 19, 34};
const char* NOME[3] = {"ESQ", "CENTRO", "DIR"};

// ---------------------------------------------------------------------------
// Inicializacao
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    pinMode(TRIG[i], OUTPUT);
    digitalWrite(TRIG[i], LOW);
    pinMode(ECHO[i], INPUT);
  }

  Serial.println("Teste HC-SR04");
}

// ---------------------------------------------------------------------------
// Leitura sequencial
// repouso deve ser 0; SEM ECO em todos indica falta de 5 V ou GND
// ---------------------------------------------------------------------------

void loop() {
  for (int i = 0; i < 3; i++) {
    int repouso = digitalRead(ECHO[i]);

    digitalWrite(TRIG[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG[i], LOW);

    unsigned long us = pulseIn(ECHO[i], HIGH, 30000);

    Serial.print(NOME[i]);
    Serial.print(" | repouso=");
    Serial.print(repouso);

    if (us == 0) {
      Serial.println(" | SEM ECO");
    } else {
      Serial.print(" | ");
      Serial.print(us / 58.0, 1);
      Serial.println(" cm");
    }

    delay(60);
  }

  Serial.println("---");
  delay(300);
}
