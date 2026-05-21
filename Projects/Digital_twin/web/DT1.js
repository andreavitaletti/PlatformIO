let client = null;
let currentTemperature = 22.0;
let currentRoomId = "incubator_labs";
let simChart = null;

// Parametri fisici reali stimati dell'incubatore (Costanti del modello)
const PHYSICS = {
  T_ambient: 20.0,
  K_cooling: 0.02, // Coefficiente di dissipazione termica
  Q_eco: 0.15, // Potenza termica modello ECO (gradi/sec)
  Q_boost: 0.45, // Potenza termica modello BOOST (gradi/sec)
  Energy_eco: 45, // Consumo energetico nominale ECO (W)
  Energy_boost: 180, // Consumo energetico nominale BOOST (W)
};

function initializeTwin() {
  currentRoomId = document.getElementById("roomId").value.trim();
  const statusText = document.getElementById("connectionStatus");
  statusText.innerText = "CONNESSIONE IN CORSO...";
  statusText.className = "text-yellow-400 font-bold font-mono";

  ////// emqx. Works in both basic WS and TLS WS:
  // const broker = 'wss://broker.emqx.io:8084/mqtt'
  // const broker = 'ws://broker.emqx.io:8083/mqtt'

  //////// shiftr.io desktop client.
  // Fill in your desktop IP address for localhost:
  // const broker = 'ws://localhost:1884';

  //////// shiftr.io, requires username and password
  // (see options variable below):
  const broker = "wss://public.cloud.shiftr.io";

  //////// test.mosquitto.org, uses no username and password:
  // const broker = 'wss://test.mosquitto.org:8081';

  // connection options:
  let options = {
    // Clean session
    clean: true,
    // connect timeout in ms:
    connectTimeout: 10000,
    // Authentication
    // add a random number for a unique client ID:
    clientId: "mqttJsClient-" + Math.floor(Math.random() * 1000000),
    // add these in for public.cloud.shiftr.io:
    username: "public",
    password: "public",
  };

  client = mqtt.connect(broker, options);

  client.on("connect", () => {
    statusText.innerText = "ONLINE";
    statusText.className = "text-emerald-400 font-bold font-mono";
    document.getElementById("connectBtn").disabled = true;
    document.getElementById("roomId").disabled = true;

    client.subscribe(`workshop/${currentRoomId}/telemetry`);
  });

  client.on("message", (topic, message) => {
    const data = JSON.parse(message.toString());
    currentTemperature = data.temperature;
    document.getElementById("liveTemp").innerText =
      `${currentTemperature.toFixed(1)}°C`;

    const modeLabel = document.getElementById("liveMode");
    if (data.heaterMode === 0) {
      modeLabel.innerText = "Profilo Attivo: ECO";
      modeLabel.className =
        "text-xs font-mono inline-block px-3 py-1 rounded bg-green-950 text-green-400 border border-green-800";
    } else {
      modeLabel.innerText = "Profilo Attivo: BOOST";
      modeLabel.className =
        "text-xs font-mono inline-block px-3 py-1 rounded bg-orange-950 text-orange-400 border border-orange-800";
    }
  });

  initChart();
}

function initChart() {
  const ctx = document.getElementById("simulationChart").getContext("2d");
  simChart = new Chart(ctx, {
    type: "line",
    data: {
      labels: Array.from({ length: 60 }, (_, i) => `${i * 3}s`),
      datasets: [
        {
          label: "Modello A (ECO)",
          data: [],
          borderColor: "#10b981",
          borderDash: [5, 5],
          tension: 0.2,
          pointRadius: 0,
        },
        {
          label: "Modello B (BOOST)",
          data: [],
          borderColor: "#f97316",
          borderDash: [5, 5],
          tension: 0.2,
          pointRadius: 0,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: { grid: { color: "#334155" }, ticks: { color: "#94a3b8" } },
        x: { grid: { display: false }, ticks: { color: "#94a3b8" } },
      },
      plugins: { legend: { labels: { color: "#f1f5f9" } } },
    },
  });
}

// Simulatore predittivo basato sul modello matematico (Eulero)
function runPredictiveSimulation() {
  const targetTemp = parseFloat(document.getElementById("targetInput").value);

  const timeStep = 3; // Orizzonte temporale a step di 3 secondi
  const maxSteps = 60; // 180 secondi totali di previsione nel futuro

  let trajEco = [];
  let trajBoost = [];

  let tEco = currentTemperature;
  let tBoost = currentTemperature;

  let ecoSettlingTime = maxSteps * timeStep;
  let boostSettlingTime = maxSteps * timeStep;

  // Esecuzione della simulazione parallela integrando nel tempo l'equazione fisica
  for (let i = 0; i < maxSteps; i++) {
    // Modello ECO
    let dT_eco =
      (PHYSICS.K_cooling * (PHYSICS.T_ambient - tEco) +
        (tEco < targetTemp ? PHYSICS.Q_eco : 0)) *
      timeStep;
    tEco += dT_eco;
    trajEco.push(tEco);
    if (tEco >= targetTemp - 0.2 && ecoSettlingTime === maxSteps * timeStep)
      ecoSettlingTime = i * timeStep;

    // Modello BOOST
    let dT_boost =
      (PHYSICS.K_cooling * (PHYSICS.T_ambient - tBoost) +
        (tBoost < targetTemp ? PHYSICS.Q_boost : 0)) *
      timeStep;
    tBoost += dT_boost;
    trajBoost.push(tBoost);
    if (tBoost >= targetTemp - 0.2 && boostSettlingTime === maxSteps * timeStep)
      boostSettlingTime = i * timeStep;
  }

  // Aggiorna il grafico con le curve generate matematicamente dal Gemello
  simChart.data.datasets[0].data = trajEco;
  simChart.data.datasets[1].data = trajBoost;
  simChart.update();

  // CALCOLO DELLA FUNZIONE DI COSTO (Ottimizzazione multicriterio)
  // J = (Tempo_impiegato) * PesoTempo + (Energia_Spesa) * PesoEnergia
  const totalEnergyEco = ecoSettlingTime * PHYSICS.Energy_eco;
  const totalEnergyBoost = boostSettlingTime * PHYSICS.Energy_boost;

  // Vincolo di fabbrica prioritario: l'incubatore NON deve impiegare più di 40 secondi
  let optimalMode = 0;
  let motivoStr = "";

  if (ecoSettlingTime > 45) {
    optimalMode = 1; // Forza BOOST per non violare i vincoli temporali di sicurezza del processo
    motivoStr = `Il modello di simulazione termica prevede che il profilo ECO violerebbe il vincolo temporale critico (>45s richiesti: stimati ${ecoSettlingTime}s). Il Gemello ha riconfigurato i parametri di attuazione forzando la modalità <strong>BOOST</strong>.`;
  } else {
    optimalMode = 0; // Rimani in ECO per minimizzare la funzione di costo energetico complessivo
    motivoStr = `La simulazione dimostra che il target è raggiungibile in modalità ECO in ${ecoSettlingTime}s (sotto la soglia critica). Parametri ottimizzati impostati su <strong>ECO</strong> per un risparmio stimato di ${((totalEnergyBoost - totalEnergyEco) / 3600).toFixed(2)} Wh.`;
  }

  document.getElementById("decisionBox").classList.remove("hidden");
  document.getElementById("decisionText").innerHTML = motivoStr;

  // Invia parametri ottimi all'ESP32
  client.publish(
    `workshop/${currentRoomId}/control`,
    JSON.stringify({
      target: targetTemp,
      heaterMode: optimalMode,
    }),
  );
}
