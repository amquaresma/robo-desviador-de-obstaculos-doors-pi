const { API_URL, ROBOT_ID, INTERVALO_MS, LIMITE_ONLINE_S } = window.DOORS_CONFIG;

const $ = (id) => document.getElementById(id);

// ---------------------------------------------------------------------------
// Utilitários de exibição
// ---------------------------------------------------------------------------

function setConexao(online) {
  const chip = $("conexao");
  chip.textContent = online ? "ONLINE" : "OFFLINE";
  chip.className = "chip " + (online ? "on" : "off");
}

function setSensor(lado, cm) {
  $("s-" + lado).textContent = cm >= 400 ? "livre" : Math.round(cm);
  const pct = Math.max(0, Math.min(100, 100 - (cm / 200) * 100));
  $("b-" + lado).style.width = pct + "%";
}

function setMotor(lado, pwm) {
  $("v-" + lado).textContent = pwm;
  $("m-" + lado).style.width = Math.round((Math.abs(pwm) / 255) * 100) + "%";
  $("m-" + lado).classList.toggle("re", pwm < 0);
}

// ---------------------------------------------------------------------------
// Leitura ao vivo
// ---------------------------------------------------------------------------

async function atualizaAoVivo() {
  try {
    const r = await fetch(`${API_URL}/robots/${ROBOT_ID}/latest`);
    if (!r.ok) throw new Error(r.status);
    const t = await r.json();

    setConexao(t.age_s < LIMITE_ONLINE_S);
    $("estado").textContent = t.robot_state ?? "—";
    $("idade").textContent = `${t.age_s.toFixed(1)} s atrás`;

    setSensor("esq", t.sensor_left);
    setSensor("cen", t.sensor_center);
    setSensor("dir", t.sensor_right);
    setMotor("esq", t.motor_left_pwm);
    setMotor("dir", t.motor_right_pwm);
  } catch {
    setConexao(false);
  }
}

// ---------------------------------------------------------------------------
// Histórico de sessões
// ---------------------------------------------------------------------------

async function atualizaSessoes() {
  try {
    const r = await fetch(`${API_URL}/sessions?robot_id=${ROBOT_ID}`);
    if (!r.ok) return;
    const sessoes = await r.json();
    const corpo = $("sessoes");

    if (!sessoes.length) {
      corpo.innerHTML = '<tr><td colspan="5">Nenhuma sessão registrada</td></tr>';
      return;
    }

    corpo.innerHTML = sessoes
      .slice(0, 10)
      .map(
        (s) => `<tr>
          <td>${s.id}</td>
          <td>${s.test_type ?? "—"}</td>
          <td>${s.total_time_s ?? "—"}</td>
          <td>${s.collisions ?? 0}</td>
          <td>${new Date(s.start_time).toLocaleString("pt-BR")}</td>
        </tr>`
      )
      .join("");
  } catch {
    /* mantém a última tabela exibida */
  }
}

// ---------------------------------------------------------------------------
// Inicialização
// ---------------------------------------------------------------------------

$("robo").textContent = ROBOT_ID;
atualizaAoVivo();
atualizaSessoes();
setInterval(atualizaAoVivo, INTERVALO_MS);
setInterval(atualizaSessoes, 10000);
