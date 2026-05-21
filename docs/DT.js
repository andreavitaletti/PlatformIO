let client = null;
let currentTemperature = 22.0; // Fallback initial temperature
let currentRoomId = "room123";

function initializeTwin() {
  currentRoomId = document.getElementById("roomId").value.trim();
  if (!currentRoomId) {
    alert("Please enter a unique Room ID!");
    return;
  }

  const statusText = document.getElementById("connectionStatus");
  statusText.innerText = "Connecting...";
  statusText.className = "text-sm font-semibold text-yellow-600";
  // Modifica la stringa di connessione in questo modo:
  var client = mqtt.connect('wss://test.mosquitto.org:8081/mqtt');
  // Use port 8884 for secure connections (wss)
  // client = mqtt.connect("wss://broker.hivemq.com:8884/mqtt");
  // Use port 8000 with ws:// instead of wss://
  //client = mqtt.connect('ws://broker.hivemq.com:8000/mqtt');
  // Connect to HiveMQ Public MQTT Broker via WebSockets
  //client = mqtt.connect("wss://broker.hivemq.com:8000/mqtt");

  client.on("connect", () => {
    statusText.innerText = "Connected";
    statusText.className = "text-sm font-semibold text-green-600";
    document.getElementById("connectBtn").disabled = true;
    document.getElementById("roomId").disabled = true;

    // Subscribe to your specific room's telemetry topic
    const telemetryTopic = `workshop/${currentRoomId}/telemetry`;
    client.subscribe(telemetryTopic);
    console.log(`Subscribed to: ${telemetryTopic}`);
  });

  // Handle incoming telemetry messages from physical/simulated ESP32
  client.on("message", (topic, message) => {
    try {
      const data = JSON.parse(message.toString());

      // Extract values from JSON payload
      currentTemperature = data.temperature;
      const mode = data.heaterMode; // 0 = Eco, 1 = Boost

      // Update UI elements
      document.getElementById("liveTemp").innerText =
        `${currentTemperature.toFixed(1)}°C`;

      const modeLabel = document.getElementById("liveMode");
      if (mode === 0) {
        modeLabel.innerText = "Active Profile: ECO";
        modeLabel.className =
          "text-sm px-3 py-1 rounded-full bg-green-100 text-green-800 font-medium";
      } else {
        modeLabel.innerText = "Active Profile: BOOST";
        modeLabel.className =
          "text-sm px-3 py-1 rounded-full bg-orange-100 text-orange-800 font-medium";
      }
    } catch (e) {
      console.error("Failed to parse telemetry JSON", e);
    }
  });

  client.on("error", (err) => {
    console.error("MQTT Connection Error: ", err);
    statusText.innerText = "Connection Failed";
    statusText.className = "text-sm font-semibold text-red-500";
  });
}

// Core Twin Feature: Parallel What-If Simulations
function runWhatIfSimulation() {
  if (!client || !client.connected) {
    alert("Please connect your Twin to the MQTT broker first!");
    return;
  }

  const targetTemp = parseFloat(document.getElementById("targetInput").value);
  const deltaT = Math.abs(targetTemp - currentTemperature);

  // --- MODEL A: ECO MODE SIMULATION ---
  // Rule: Slow ramp up, highly energy-efficient
  const ecoTimeRequired = deltaT * 4.0; // 4 seconds per degree
  const ecoEnergySpent = deltaT * 0.8; // 0.8 Wh per degree

  // --- MODEL B: BOOST MODE SIMULATION ---
  // Rule: Quick response, heavy power overhead
  const boostTimeRequired = deltaT * 1.5; // 1.5 seconds per degree
  const boostEnergySpent = deltaT * 2.5; // 2.5 Wh per degree

  // Update Digital Twin Simulation Panel
  document.getElementById("ecoTime").innerText =
    `Time: ${ecoTimeRequired.toFixed(1)}s`;
  document.getElementById("ecoEnergy").innerText =
    `Est. Energy: ${ecoEnergySpent.toFixed(1)} Wh`;
  // Max scale visualized at roughly 100s for ui bar
  document.getElementById("ecoBar").style.width =
    `${Math.min(ecoTimeRequired, 100)}%`;

  document.getElementById("boostTime").innerText =
    `Time: ${boostTimeRequired.toFixed(1)}s`;
  document.getElementById("boostEnergy").innerText =
    `Est. Energy: ${boostEnergySpent.toFixed(1)} Wh`;
  document.getElementById("boostBar").style.width =
    `${Math.min(boostTimeRequired, 100)}%`;

  // --- OPTIMIZATION ENGINE & CONSTRAINTS ---
  let optimalMode = 0; // Default parameters to ECO
  let optimizationReason = "";

  // Goal/Constraint: If target takes more than 30 seconds via Eco,
  // it risks breaking system constraints. Trade efficiency for speed.
  if (ecoTimeRequired > 30.0) {
    optimalMode = 1; // Pick BOOST
    optimizationReason = `Eco mode would take too long (${ecoTimeRequired.toFixed(1)}s). Switched system profile to <strong>BOOST Mode</strong> to satisfy time constraint (<30s).`;
  } else {
    optimalMode = 0; // Pick ECO
    optimizationReason = `Eco mode can reach the target safely within ${ecoTimeRequired.toFixed(1)}s. Selected <strong>ECO Mode</strong> to minimize energy footprint.`;
  }

  // Display Strategy Decision to User
  const dBox = document.getElementById("decisionBox");
  const dText = document.getElementById("decisionText");
  dBox.classList.remove("hidden");
  dText.innerHTML = optimizationReason;

  // --- EXECUTE FEEDBACK LOOP ---
  // Package selected parameters and dispatch down to physical device
  const commandPayload = {
    target: targetTemp,
    heaterMode: optimalMode,
  };

  const controlTopic = `workshop/${currentRoomId}/control`;
  client.publish(controlTopic, JSON.stringify(commandPayload));
  console.log(
    `Dispatched configuration parameters to ${controlTopic}:`,
    commandPayload,
  );
}
