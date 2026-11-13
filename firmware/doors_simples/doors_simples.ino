/*
  DOORS - Versao simples: anda e desvia, sem estacionar
  ESP32 WROOM-32 | TB6612FNG | 2x N20 6V | 3x HC-SR04
  ESP32 Arduino Core 3.x | Serial 115200
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

#define TRIG_C 18
#define ECHO_C 19
#define TRIG_E 21
#define ECHO_E 22
#define TRIG_D 23
#define ECHO_D 34

// ---------------------------------------------------------------------------
// Ajustes
// ---------------------------------------------------------------------------

const int VEL_RETA  = 180;   // velocidade em linha reta (0 a 255)
const int VEL_CURVA = 150;   // velocidade da roda de fora na curva
const int VEL_GIRO  = 150;   // velocidade no giro de emergencia

const int DIST_ATENCAO = 60; // cm: comeca a curvar
const int DIST_PERIGO  = 25; // cm: curva fechada, roda de dentro para tras
const int DIST_PAREDE  = 15; // cm: afasta das barreiras laterais

const bool INVERTE_ESQ = false;
const bool INVERTE_DIR = false;

// ---------------------------------------------------------------------------
// Sensores
// Sem eco dentro do tempo limite = caminho livre.
// ---------------------------------------------------------------------------

const unsigned long TIMEOUT_US = 12000;  // ~200 cm
const int SEM_OBSTACULO = 200;

int lerSensor(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  unsigned long us = pulseIn(echo, HIGH, TIMEOUT_US);
  if (us == 0) return SEM_OBSTACULO;

  int cm = us / 58;
  if (cm < 2) return SEM_OBSTACULO;
  return cm;
}

// ---------------------------------------------------------------------------
// Motores
// Velocidade de -255 (re) a 255 (frente).
// ---------------------------------------------------------------------------

void motor(int in1, int in2, int pwm, int v, bool inverte) {
  if (inverte) v = -v;
  v = constrain(v, -255, 255);

  digitalWrite(in1, v > 0 ? HIGH : LOW);
  digitalWrite(in2, v < 0 ? HIGH : LOW);
  ledcWrite(pwm, abs(v));
}

void rodas(int esq, int dir) {
  motor(AIN1, AIN2, PWMA, esq, INVERTE_ESQ);
  motor(BIN1, BIN2, PWMB, dir, INVERTE_DIR);
}

// ---------------------------------------------------------------------------
// Inicializacao
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  int saidas[] = {STBY, AIN1, AIN2, BIN1, BIN2, TRIG_C, TRIG_E, TRIG_D};
  for (int p : saidas) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }
  pinMode(ECHO_C, INPUT);
  pinMode(ECHO_E, INPUT);
  pinMode(ECHO_D, INPUT);

  ledcAttach(PWMA, 25000, 8);
  ledcAttach(PWMB, 25000, 8);

  rodas(0, 0);
  delay(2000);
  digitalWrite(STBY, HIGH);
}

// ---------------------------------------------------------------------------
// Loop: le os 3 sensores e decide a direcao
// ---------------------------------------------------------------------------

void loop() {
  int e = lerSensor(TRIG_E, ECHO_E);
  delay(10);
  int c = lerSensor(TRIG_C, ECHO_C);
  delay(10);
  int d = lerSensor(TRIG_D, ECHO_D);

  bool esquerdaMaisLivre = (e >= d);
  const char* acao;

  if (c < DIST_PERIGO) {
    // Obstaculo colado: gira para o lado mais livre
    if (esquerdaMaisLivre) {
      rodas(-VEL_GIRO, VEL_GIRO);
      acao = "GIRO ESQ";
    } else {
      rodas(VEL_GIRO, -VEL_GIRO);
      acao = "GIRO DIR";
    }
  } else if (c < DIST_ATENCAO) {
    // Obstaculo se aproximando: curva tanto mais fechada quanto mais perto
    int dentro = map(c, DIST_PERIGO, DIST_ATENCAO, 0, VEL_CURVA);
    if (esquerdaMaisLivre) {
      rodas(dentro, VEL_CURVA);
      acao = "CURVA ESQ";
    } else {
      rodas(VEL_CURVA, dentro);
      acao = "CURVA DIR";
    }
  } else if (e < DIST_PAREDE) {
    // Barreira encostando na esquerda: afasta para a direita
    rodas(VEL_RETA, VEL_RETA / 2);
    acao = "AFASTA DIR";
  } else if (d < DIST_PAREDE) {
    // Barreira encostando na direita: afasta para a esquerda
    rodas(VEL_RETA / 2, VEL_RETA);
    acao = "AFASTA ESQ";
  } else {
    rodas(VEL_RETA, VEL_RETA);
    acao = "RETA";
  }

  Serial.print("E:");
  Serial.print(e);
  Serial.print("  C:");
  Serial.print(c);
  Serial.print("  D:");
  Serial.print(d);
  Serial.print("  | ");
  Serial.println(acao);
}
