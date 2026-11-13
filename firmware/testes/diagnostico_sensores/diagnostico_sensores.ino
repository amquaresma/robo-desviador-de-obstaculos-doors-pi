/*
  DOORS - Diagnostico dos 3 HC-SR04
  ESP32 Arduino Core 3.x | Serial 115200
  Coloque a mao a ~15 cm de cada sensor. A bateria precisa estar ligada.
*/

// ---------------------------------------------------------------------------
// Pinos (esquerdo, centro, direito)
// ---------------------------------------------------------------------------

const int TRIG[3] = {21, 18, 23};
const int ECHO[3] = {22, 19, 34};
const char* NOME[3] = {"ESQ", "CENTRO", "DIR"};

// ---------------------------------------------------------------------------
// Inicializacao: ponte H desligada durante o teste
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  for (int i = 0; i < 3; i++) {
    pinMode(TRIG[i], OUTPUT);
    digitalWrite(TRIG[i], LOW);
    pinMode(ECHO[i], INPUT);
  }
}

// ---------------------------------------------------------------------------
// Observa o pino ECHO por 40 ms e mostra o que aconteceu
// NUNCA SUBIU      -> sem 5 V, sem GND comum ou divisor invertido
// SUBIU E NAO DESCEU -> GPIO ligado no lugar errado
// repouso=1        -> ECHO preso em nivel alto
// ---------------------------------------------------------------------------

void loop() {
  for (int i = 0; i < 3; i++) {
    int repouso = digitalRead(ECHO[i]);

    digitalWrite(TRIG[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG[i], LOW);

    unsigned long inicio = micros();
    unsigned long subida = 0;
    unsigned long descida = 0;

    while (micros() - inicio < 40000) {
      int nivel = digitalRead(ECHO[i]);
      if (nivel == HIGH && subida == 0) subida = micros();
      if (nivel == LOW && subida != 0 && descida == 0) descida = micros();
    }

    Serial.print(NOME[i]);
    Serial.print(" | repouso=");
    Serial.print(repouso);

    if (subida == 0) {
      Serial.println(" | ECHO NUNCA SUBIU");
    } else if (descida == 0) {
      Serial.println(" | ECHO SUBIU E NAO DESCEU");
    } else {
      unsigned long us = descida - subida;
      Serial.print(" | ");
      Serial.print(us);
      Serial.print(" us = ");
      Serial.print(us / 58.0, 1);
      Serial.println(" cm");
    }
    delay(60);
  }

  Serial.println("---");
  delay(400);
}
