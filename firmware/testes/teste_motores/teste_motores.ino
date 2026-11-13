/*
  DOORS - Teste isolado dos motores (TB6612FNG)
  ESP32 Arduino Core 3.x | Serial 115200
  Rodar com o carrinho suspenso.
*/

// ---------------------------------------------------------------------------
// Pinos
// ---------------------------------------------------------------------------

#define AIN1 25
#define AIN2 26
#define PWMA 27

#define BIN1 32
#define BIN2 33
#define PWMB 14

#define STBY 13

const int VEL = 180;

// ---------------------------------------------------------------------------
// Controle dos motores
// ---------------------------------------------------------------------------

void motor(int in1, int in2, int pwm, int v) {
  digitalWrite(in1, v > 0 ? HIGH : LOW);
  digitalWrite(in2, v < 0 ? HIGH : LOW);
  ledcWrite(pwm, abs(v));
}

void parar() {
  motor(AIN1, AIN2, PWMA, 0);
  motor(BIN1, BIN2, PWMB, 0);
}

void passo(const char* texto, int vEsq, int vDir) {
  Serial.println(texto);
  motor(AIN1, AIN2, PWMA, vEsq);
  motor(BIN1, BIN2, PWMB, vDir);
  delay(2000);
  parar();
  delay(1000);
}

// ---------------------------------------------------------------------------
// Inicializacao
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  int saidas[] = {STBY, AIN1, AIN2, BIN1, BIN2};
  for (int p : saidas) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }

  ledcAttach(PWMA, 1000, 8);
  ledcAttach(PWMB, 1000, 8);

  parar();
  digitalWrite(STBY, HIGH);
  Serial.println("Teste de motores");
}

// ---------------------------------------------------------------------------
// Sequencia de teste
// ---------------------------------------------------------------------------

void loop() {
  passo("Esquerdo frente", VEL, 0);
  passo("Esquerdo re", -VEL, 0);
  passo("Direito frente", 0, VEL);
  passo("Direito re", 0, -VEL);
  passo("Os dois frente", VEL, VEL);
}
