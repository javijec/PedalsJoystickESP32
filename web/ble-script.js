let bleDevice;
let bleServer;
let bleService;
let rxCharacteristic;
let txCharacteristic;

const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"; // Mobile writes to this
const TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"; // Mobile receives from this

const connectBtn = document.getElementById("connectBtn");
const calibBtn = document.getElementById("calibBtn");
const diagBtn = document.getElementById("diagBtn");
const monitorBtn = document.getElementById("monitorBtn");
const advancedToggle = document.getElementById("advancedToggle");
const advancedSection = document.getElementById("advancedSection");
const resetBtn = document.getElementById("resetBtn");
const statusDot = document.getElementById("statusDot");
const statusText = document.getElementById("statusText");
const log = document.getElementById("log");
const calibModal = document.getElementById("calibModal");
const modalPedalName = document.getElementById("modalPedalName");
const modalStepText = document.getElementById("modalStepText");
const modalNextBtn = document.getElementById("modalNextBtn");

const gasBar = document.getElementById("gasBar");
const brakeBar = document.getElementById("brakeBar");
const clutchBar = document.getElementById("clutchBar");
const gasVal = document.getElementById("gasVal");
const brakeVal = document.getElementById("brakeVal");
const clutchVal = document.getElementById("clutchVal");

const gMin = document.getElementById("g-min");
const gMax = document.getElementById("g-max");
const bMax = document.getElementById("b-max");
const bScale = document.getElementById("b-scale");
const cMax = document.getElementById("c-max");

const rawG = document.getElementById("raw-g");
const rawB = document.getElementById("raw-b");
const rawC = document.getElementById("raw-c");

async function connect() {
  try {
    appendLog("Requesting Bluetooth Device...");
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ name: "PedalMaster BLE" }],
      optionalServices: [SERVICE_UUID],
    });

    bleDevice.addEventListener("gattserverdisconnected", onDisconnected);

    appendLog("Connecting to GATT Server...");
    bleServer = await bleDevice.gatt.connect();

    appendLog("Getting Service...");
    bleService = await bleServer.getPrimaryService(SERVICE_UUID);

    appendLog("Getting Characteristics...");
    rxCharacteristic = await bleService.getCharacteristic(RX_UUID);
    txCharacteristic = await bleService.getCharacteristic(TX_UUID);

    await txCharacteristic.startNotifications();
    txCharacteristic.addEventListener(
      "characteristicvaluechanged",
      handleNotifications
    );

    statusDot.classList.add("connected");
    statusText.innerText = "BLE CONNECTED";
    connectBtn.innerText = "DISCONNECT";
    setUIEnabled(true);

    appendLog("Connected to PedalMaster!");

    // Request initial calibration data
    sendCommand("m");
  } catch (e) {
    appendLog("Error: " + e.message);
  }
}

function onDisconnected() {
  statusDot.classList.remove("connected");
  statusText.innerText = "BLE OFFLINE";
  connectBtn.innerText = "CONNECT BLUETOOTH";
  setUIEnabled(false);
  appendLog("Disconnected from device.");
}

function setUIEnabled(enabled) {
  calibBtn.disabled = !enabled;
  diagBtn.disabled = !enabled;
  monitorBtn.disabled = !enabled;
  advancedToggle.disabled = !enabled;
  resetBtn.disabled = !enabled;
}

let bleBuffer = "";
function handleNotifications(event) {
  const value = new TextDecoder().decode(event.target.value);
  bleBuffer += value;

  let lines = bleBuffer.split("\n");
  bleBuffer = lines.pop();

  for (let line of lines) {
    line = line.trim();
    if (line.startsWith("{")) {
      try {
        const data = JSON.parse(line);
        updateUI(data);
      } catch (e) {}
    } else if (line.length > 0) {
      appendLog(line);
    }
  }
}

function updateUI(data) {
  if (data.g !== undefined) {
    const gasPct = (data.g / 4095) * 100;
    gasBar.style.width = `${gasPct}%`;
    gasVal.innerText = `${Math.round(gasPct)}%`;
  }
  if (data.b !== undefined) {
    const brakePct = (data.b / 16384) * 100;
    brakeBar.style.width = `${brakePct}%`;
    brakeVal.innerText = `${Math.round(brakePct)}%`;
  }
  if (data.c !== undefined) {
    const clutchPct = (data.c / 4095) * 100;
    clutchBar.style.width = `${clutchPct}%`;
    clutchVal.innerText = `${Math.round(clutchPct)}%`;
  }

  if (data.rg !== undefined && rawG) rawG.innerText = data.rg;
  if (data.rb !== undefined && rawB) rawB.innerText = data.rb.toFixed(0);
  if (data.rc !== undefined && rawC) rawC.innerText = data.rc;

  if (data.cal) {
    gMin.innerText = data.cal.gmin;
    gMax.innerText = data.cal.gmax;
    bMax.innerText = data.cal.bmax.toFixed(0);
    if (data.cal.bmax > 0)
      bScale.innerText = (16384 / data.cal.bmax).toFixed(4);
    cMin.innerText = data.cal.cmin;
    cMax.innerText = data.cal.cmax;

    // Al recibir datos de calibración, ocultamos el modal
    hideCalibrationModal();
  }
}

function showCalibrationModal(title) {
  modalPedalName.innerText = title;
  modalStepText.innerText = "Sigue las instrucciones en la pantalla del ESP32.";
  calibModal.classList.add("active");
}

function hideCalibrationModal() {
  calibModal.classList.remove("active");
}

async function calibrateIndividual(pedal) {
  if (document.activeElement) document.activeElement.blur();
  let command = "";
  let name = "";
  if (pedal === "g") {
    command = "g";
    name = "ACELERADOR";
  } else if (pedal === "b") {
    command = "b";
    name = "FRENO";
  } else if (pedal === "c") {
    command = "e";
    name = "EMBRAGUE";
  }

  if (command) {
    showCalibrationModal(name);
    await sendCommand(command);
    appendLog(`Iniciando calibración individual para ${name}...`);
  }
}

async function sendCommand(cmd) {
  if (!rxCharacteristic) return;
  const encoder = new TextEncoder();
  await rxCharacteristic.writeValue(encoder.encode(cmd + "\n"));
}

function appendLog(msg) {
  const div = document.createElement("div");
  div.innerText = `> ${msg}`;
  log.appendChild(div);
  log.scrollTop = log.scrollHeight;
}

connectBtn.addEventListener("click", () => {
  if (bleDevice && bleDevice.gatt.connected) {
    bleDevice.gatt.disconnect();
  } else {
    connect();
  }
});

calibBtn.addEventListener("click", () => {
  showCalibrationModal("CALIBRACION COMPLETA");
  sendCommand("c");
  appendLog("Starting calibration...");
});

modalNextBtn.addEventListener("click", () => {
  sendCommand("\n");
});

window.addEventListener("keydown", (e) => {
  if (calibModal.classList.contains("active") && e.key === "Enter") {
    sendCommand("\n");
  }
});

diagBtn.addEventListener("click", () => {
  sendCommand("d");
  appendLog("Requesting hardware diagnostics...");
});

monitorBtn.addEventListener("click", () => {
  sendCommand("m");
  appendLog("Syncing data...");
});

async function saveDZ(pedal) {
  // Función desactivada
}

advancedToggle.addEventListener("click", () => {
  advancedSection.classList.toggle("visible");
  advancedToggle.innerText = advancedSection.classList.contains("visible")
    ? "HIDE TOOLS"
    : "TOOLS";
});

resetBtn.addEventListener("click", async () => {
  if (confirm("Reset to factory defaults?")) {
    await sendCommand("r");
    appendLog("Factory reset sent.");
  }
});
