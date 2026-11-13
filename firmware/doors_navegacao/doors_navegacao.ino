/*
  DOORS - Veiculo autonomo desviador de obstaculos
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
#define BOTAO_BOOT 0

#define TRIG_C 18
#define ECHO_C 19
#define TRIG_E 21
#define ECHO_E 22
#define TRIG_D 23
#define ECHO_D 34

// ---------------------------------------------------------------------------
// Geometria do carrinho (medir no robo real)
// ---------------------------------------------------------------------------

const float ANG_LATERAL_GRAUS = 20.0;  // inclinacao dos sensores laterais
const float BITOLA_CM         = 15.0;  // distancia entre o centro das duas rodas
const float MEIA_LARGURA_CM   = 9.0;   // metade da largura total do carrinho
const float FOLGA_CM          = 6.0;   // margem extra para nao raspar

// ---------------------------------------------------------------------------
// Calibracao dos motores
// ---------------------------------------------------------------------------

const bool INVERTE_ESQ = false;
const bool INVERTE_DIR = false;

const int V_MAX    = 180;
const int V_CURVA  = 140;
const int V_PIVO   = 150;
const int V_MINIMO = 85;

const float FATOR_ESQ = 1.00;
const float FATOR_DIR = 1.00;

// ---------------------------------------------------------------------------
// Parametros do desvio
// ---------------------------------------------------------------------------

const float D_ANTECIPA    = 120;   // cm: comeca a planejar o desvio
const float D_SEGURANCA   = 8;     // cm: a curva precisa terminar antes disso
const float S_MINIMO      = 5;     // cm: menor trecho considerado para girar
const float W_PASSAGEM    = 30;    // cm: deslocamento lateral para contornar o obstaculo
const float D_SEM_LEITURA = 40;    // cm: valor usado quando um sensor falha
const float HISTERESE     = 15;
const float TROCA_LADO    = 30;

const float Y_TETO_CM   = 30;      // lateral acima disso nao influencia a centralizacao
const float K_CENTRO    = 0.0005;  // 1/cm^2: forca de centralizacao entre as barreiras
const float KAPPA_CURVA = 0.02;    // 1/cm: acima disso o estado vira CURVA

// ---------------------------------------------------------------------------
// Estacionamento: motores so param com os 3 sensores abaixo de 5 cm
// ---------------------------------------------------------------------------

const float DIST_ESTACIONAR_CM = 5.0;
const unsigned long CONFIRMA_CERCADO_MS = 500;

const float DIST_SEM_OBSTACULO_CM = 200.0;
const unsigned long TEMPO_SEM_OBSTACULO_MS = 6000;
const bool ATIVA_REGRA_6S = false;

const unsigned long ATRASO_PARTIDA_MS = 2000;

// ---------------------------------------------------------------------------
// Sensores ultrassonicos
// Leitura por interrupcao: o loop nunca espera o eco.
// Eco que sobe e nao volta a tempo = caminho livre.
// Eco que nem sobe = falha de ligacao do sensor.
// Quedas que duram menos que GLITCH_US sao ruido dos motores e sao ignoradas.
// Cada distancia e a mediana das 3 ultimas leituras do sensor.
// ---------------------------------------------------------------------------

const unsigned long INTERVALO_TRIGGER_MS = 20;
const unsigned long GLITCH_US = 40;
const unsigned long ECO_MIN_US = 120;
const unsigned long ECO_MAX_US = 23500;
const float DIST_MAX = 400.0;
const uint8_t FALHAS_PARA_INVALIDO = 6;

struct Sonar {
  const char* nome;
  int trig;
  int echo;
  volatile uint32_t tSubida;
  volatile uint32_t tQueda;
  volatile bool subiu;
  volatile bool caiu;
  volatile bool pronto;
  bool medido;
  float hist[3];
  uint8_t ih;
  float cm;
  bool valido;
  uint8_t falhas;
};

Sonar sC = {"C", TRIG_C, ECHO_C, 0, 0, false, false, false, true, {DIST_MAX, DIST_MAX, DIST_MAX}, 0, DIST_MAX, true, 0};
Sonar sE = {"E", TRIG_E, ECHO_E, 0, 0, false, false, false, true, {DIST_MAX, DIST_MAX, DIST_MAX}, 0, DIST_MAX, true, 0};
Sonar sD = {"D", TRIG_D, ECHO_D, 0, 0, false, false, false, true, {DIST_MAX, DIST_MAX, DIST_MAX}, 0, DIST_MAX, true, 0};
Sonar* sonares[3] = {&sE, &sC, &sD};

void IRAM_ATTR trataEco(Sonar* s) {
  if (s->pronto) return;

  uint32_t t = micros();

  if (digitalRead(s->echo)) {
    if (!s->subiu) {
      s->tSubida = t;
      s->subiu = true;
    }
    s->caiu = false;
  } else if (s->subiu) {
    s->tQueda = t;
    s->caiu = true;
  }
}

void IRAM_ATTR isrC() { trataEco(&sC); }
void IRAM_ATTR isrE() { trataEco(&sE); }
void IRAM_ATTR isrD() { trataEco(&sD); }

float mediana3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

void registra(Sonar* s, float cm) {
  s->hist[s->ih] = cm;
  s->ih = (s->ih + 1) % 3;
  s->cm = mediana3(s->hist[0], s->hist[1], s->hist[2]);
  s->valido = true;
  s->falhas = 0;
}

void processaLeituras() {
  for (Sonar* s : sonares) {
    if (s->medido) continue;

    noInterrupts();
    bool caiu = s->caiu;
    uint32_t tSubida = s->tSubida;
    uint32_t tQueda = s->tQueda;
    interrupts();

    // Queda confirmada: o sinal ficou baixo por mais que GLITCH_US
    if (caiu && micros() - tQueda >= GLITCH_US && digitalRead(s->echo) == LOW) {
      s->pronto = true;
      s->medido = true;

      uint32_t duracao = tQueda - tSubida;
      if (duracao < ECO_MIN_US) continue;

      registra(s, (duracao >= ECO_MAX_US) ? DIST_MAX : duracao / 58.0);
    }
  }
}

void fechaCiclo(Sonar* s) {
  if (s->medido) return;

  // Modulo ainda ocupado com o eco anterior: mantem a ultima leitura
  if (!s->subiu && digitalRead(s->echo) == HIGH) return;

  if (s->subiu) {
    registra(s, DIST_MAX);
  } else if (++s->falhas >= FALHAS_PARA_INVALIDO) {
    s->valido = false;
  }
}

void dispara(Sonar* s) {
  fechaCiclo(s);

  noInterrupts();
  s->subiu = false;
  s->caiu = false;
  s->pronto = false;
  interrupts();
  s->medido = false;

  digitalWrite(s->trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(s->trig, LOW);
}

uint8_t vez = 0;
unsigned long ultimoTrigger = 0;

void atualizaSensores() {
  processaLeituras();

  if (millis() - ultimoTrigger >= INTERVALO_TRIGGER_MS) {
    ultimoTrigger = millis();
    dispara(sonares[vez]);
    vez = (vez + 1) % 3;
  }
}

float distancia(const Sonar& s) {
  return s.valido ? s.cm : D_SEM_LEITURA;
}

// ---------------------------------------------------------------------------
// Motores
// Velocidade de -255 (re) a 255 (frente).
// ---------------------------------------------------------------------------

int pwmEsq = 0;
int pwmDir = 0;

void motor(int in1, int in2, int pwm, int v, bool inverte, float fator) {
  if (inverte) v = -v;

  int a = abs(v) * fator;
  if (a < 40) {
    a = 0;
  } else if (a < V_MINIMO) {
    a = V_MINIMO;
  }
  a = constrain(a, 0, 255);

  digitalWrite(in1, v > 0 ? HIGH : LOW);
  digitalWrite(in2, v < 0 ? HIGH : LOW);
  ledcWrite(pwm, a);
}

void rodas(int vEsq, int vDir) {
  pwmEsq = vEsq;
  pwmDir = vDir;
  motor(AIN1, AIN2, PWMA, vEsq, INVERTE_ESQ, FATOR_ESQ);
  motor(BIN1, BIN2, PWMB, vDir, INVERTE_DIR, FATOR_DIR);
}

void desligaMotores() {
  rodas(0, 0);
  digitalWrite(STBY, LOW);
}

// ---------------------------------------------------------------------------
// Estado e estacionamento
// ---------------------------------------------------------------------------

enum Estado { INICIANDO, ANDANDO, CURVA_ESQ, CURVA_DIR, PIVO_ESQ, PIVO_DIR, ESTACIONADO };

const char* NOME_ESTADO[] = {
  "INICIANDO", "ANDANDO", "CURVA_ESQ", "CURVA_DIR", "PIVO_ESQ", "PIVO_DIR", "ESTACIONADO"
};

Estado estado = INICIANDO;
int lado = 0;

unsigned long tPartida = 0;
unsigned long tCercado = 0;
unsigned long tSemObstaculo = 0;

float frenteLog = 0;
float anguloLog = 0;
float raioLog = 0;

void estaciona(const char* motivo) {
  desligaMotores();
  estado = ESTACIONADO;
  Serial.print("ESTACIONADO: ");
  Serial.println(motivo);
  Serial.println("Aperte BOOT para recomecar");
}

bool verificaEstacionamento(unsigned long agora) {
  bool cercado = sC.valido && sE.valido && sD.valido &&
                 sC.cm < DIST_ESTACIONAR_CM &&
                 sE.cm < DIST_ESTACIONAR_CM &&
                 sD.cm < DIST_ESTACIONAR_CM;

  if (cercado) {
    if (tCercado == 0) tCercado = agora;
    if (agora - tCercado >= CONFIRMA_CERCADO_MS) {
      estaciona("cercado pelos 3 lados");
      return true;
    }
  } else {
    tCercado = 0;
  }

  bool frenteLivre = ATIVA_REGRA_6S && sC.valido && sC.cm >= DIST_SEM_OBSTACULO_CM;

  if (frenteLivre) {
    if (tSemObstaculo == 0) tSemObstaculo = agora;
    if (agora - tSemObstaculo >= TEMPO_SEM_OBSTACULO_MS) {
      estaciona("6 s sem obstaculo a frente");
      return true;
    }
  } else {
    tSemObstaculo = 0;
  }

  return false;
}

// ---------------------------------------------------------------------------
// Navegacao geometrica
//
// 1. Cada leitura vira um ponto no plano do carrinho (x para frente,
//    y para a esquerda): x = d.cos(theta), y = d.sin(theta).
// 2. Distancia a frente = menor x entre os pontos que estao na faixa
//    ocupada pelo corpo do carrinho (|y| < meia largura).
// 3. Angulo de desvio necessario: phi = atan(W_PASSAGEM / frente).
// 4. Curvatura para completar o desvio antes do obstaculo:
//    kappa = phi / (frente - D_SEGURANCA). Raio da curva R = 1 / kappa.
// 5. Cinematica diferencial: vEsq = v.(1 - kappa.B/2), vDir = v.(1 + kappa.B/2).
//    Se kappa.B/2 >= 1 a roda de dentro inverte: giro no proprio eixo.
// ---------------------------------------------------------------------------

const float SEN_LAT = sin(ANG_LATERAL_GRAUS * DEG_TO_RAD);
const float COS_LAT = cos(ANG_LATERAL_GRAUS * DEG_TO_RAD);
const float CORREDOR_CM = MEIA_LARGURA_CM + FOLGA_CM;
const float KAPPA_PIVO = 2.0 / BITOLA_CM;

// Curvatura que afasta o carrinho de um ponto lateral dentro da folga
float curvaturaLateral(float x, float y) {
  if (y >= CORREDOR_CM) return 0;
  float phi = atan2(CORREDOR_CM - y, x);
  return phi / max(x - D_SEGURANCA, S_MINIMO);
}

void navega() {
  unsigned long agora = millis();

  if (verificaEstacionamento(agora)) return;

  float c = distancia(sC);
  float e = distancia(sE);
  float d = distancia(sD);

  // Pontos dos sensores laterais no plano do carrinho
  float xE = e * COS_LAT;
  float yE = e * SEN_LAT;
  float xD = d * COS_LAT;
  float yD = d * SEN_LAT;

  // Obstaculo mais proximo na faixa ocupada pelo corpo do carrinho
  float frente = c;
  if (yE < MEIA_LARGURA_CM) frente = min(frente, xE);
  if (yD < MEIA_LARGURA_CM) frente = min(frente, xD);

  // Lado do desvio: o mais livre; so troca se o outro ficar bem melhor
  if (frente < D_ANTECIPA) {
    if (lado == 0) {
      lado = (e >= d) ? 1 : -1;
    } else if (lado > 0 && d > e + TROCA_LADO) {
      lado = -1;
    } else if (lado < 0 && e > d + TROCA_LADO) {
      lado = 1;
    }
  } else if (frente > D_ANTECIPA + HISTERESE) {
    lado = 0;
  }

  // Curvatura total (1/cm), positiva para a esquerda
  float kappa = 0;
  float phi = 0;

  if (lado != 0) {
    phi = atan2(W_PASSAGEM, frente);
    kappa = lado * phi / max(frente - D_SEGURANCA, S_MINIMO);
  } else {
    float yEc = min(yE, Y_TETO_CM);
    float yDc = min(yD, Y_TETO_CM);
    kappa = K_CENTRO * (yEc - yDc);
  }

  kappa -= curvaturaLateral(xE, yE);
  kappa += curvaturaLateral(xD, yD);

  // Velocidade linear cai conforme o obstaculo se aproxima
  float u = (D_ANTECIPA - frente) / (D_ANTECIPA - D_SEGURANCA);
  u = constrain(u, 0.0f, 1.0f);
  float v = V_MAX - (V_MAX - V_CURVA) * u;

  // Cinematica diferencial
  int vEsq;
  int vDir;

  if (fabs(kappa) >= KAPPA_PIVO) {
    int sentido = (kappa > 0) ? 1 : -1;
    vEsq = -sentido * V_PIVO;
    vDir =  sentido * V_PIVO;
    estado = (sentido > 0) ? PIVO_ESQ : PIVO_DIR;
  } else {
    float fEsq = 1.0 - kappa * BITOLA_CM / 2.0;
    float fDir = 1.0 + kappa * BITOLA_CM / 2.0;
    float escala = V_MAX / (v * max(fabs(fEsq), fabs(fDir)));
    if (escala > 1.0) escala = 1.0;

    vEsq = v * fEsq * escala;
    vDir = v * fDir * escala;

    if (fabs(kappa) < KAPPA_CURVA) {
      estado = ANDANDO;
    } else {
      estado = (kappa > 0) ? CURVA_ESQ : CURVA_DIR;
    }
  }

  frenteLog = frente;
  anguloLog = phi * RAD_TO_DEG * lado;
  raioLog = (fabs(kappa) > 0.0001) ? 1.0 / kappa : 0;

  rodas(vEsq, vDir);
}

// ---------------------------------------------------------------------------
// Monitor serial
// ---------------------------------------------------------------------------

unsigned long ultimoLog = 0;

void imprimeLog() {
  if (millis() - ultimoLog < 100) return;
  ultimoLog = millis();

  for (Sonar* s : sonares) {
    Serial.print(s->nome);
    Serial.print(":");
    if (s->valido) {
      Serial.print(s->cm, 0);
    } else {
      Serial.print("ERRO");
    }
    Serial.print("  ");
  }

  Serial.print("| frente:");
  Serial.print(frenteLog, 0);
  Serial.print(" ang:");
  Serial.print(anguloLog, 0);
  Serial.print(" R:");
  Serial.print(raioLog, 0);
  Serial.print(" | ");
  Serial.print(NOME_ESTADO[estado]);
  Serial.print(" | E:");
  Serial.print(pwmEsq);
  Serial.print(" D:");
  Serial.println(pwmDir);
}

// ---------------------------------------------------------------------------
// Inicializacao e loop principal
// ---------------------------------------------------------------------------

void iniciaPercurso() {
  lado = 0;
  tCercado = 0;
  tSemObstaculo = 0;
  estado = INICIANDO;
  tPartida = millis();
  Serial.println("Partida em 2 s");
}

void setup() {
  Serial.begin(115200);

  int saidas[] = {STBY, AIN1, AIN2, BIN1, BIN2, TRIG_C, TRIG_E, TRIG_D};
  for (int p : saidas) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }

  if (!ledcAttach(PWMA, 25000, 8) || !ledcAttach(PWMB, 25000, 8)) {
    Serial.println("ERRO NO PWM");
    while (true) delay(1000);
  }

  pinMode(ECHO_C, INPUT);
  pinMode(ECHO_E, INPUT);
  pinMode(ECHO_D, INPUT);
  pinMode(BOTAO_BOOT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ECHO_C), isrC, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ECHO_E), isrE, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ECHO_D), isrD, CHANGE);

  Serial.println("DOORS - navegacao com 3 sensores");
  iniciaPercurso();
}

void loop() {
  atualizaSensores();

  switch (estado) {
    case ESTACIONADO:
      if (digitalRead(BOTAO_BOOT) == LOW) {
        delay(300);
        iniciaPercurso();
      }
      break;

    case INICIANDO:
      if (millis() - tPartida >= ATRASO_PARTIDA_MS) {
        digitalWrite(STBY, HIGH);
        estado = ANDANDO;
      }
      break;

    default:
      navega();
      break;
  }

  imprimeLog();
}
