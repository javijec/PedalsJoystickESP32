let port; // Serial Port
let reader; // Serial Reader
let bleDevice; // BLE Device
let rxCharacteristic; // BLE RX
let txCharacteristic; // BLE TX
let connectionMode = null; // 'serial' or 'ble'

const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

// UI Elements
const connectSerialBtn = document.getElementById("connectSerialBtn");
const connectBleBtn = document.getElementById("connectBleBtn");
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
const disconnectBtn = document.getElementById("disconnectBtn");
const connectOnlyElements = document.querySelectorAll(".connect-only");
const offlineOnlyElements = document.querySelectorAll(".offline-only");

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
const cMin = document.getElementById("c-min");
const cMax = document.getElementById("c-max");

const rawG = document.getElementById("raw-g");
const rawB = document.getElementById("raw-b");
const rawC = document.getElementById("raw-c");

// --- Connection Logic ---

async function connectSerial() {
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });
    connectionMode = "serial";
    onConnected("SERIAL ONLINE");
    readLoopSerial();
  } catch (e) {
    appendLog("Serial Error: " + e.message);
  }
}

async function connectBLE() {
  try {
    appendLog("Requesting Bluetooth Device...");
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ name: "PedalMaster BLE" }],
      optionalServices: [SERVICE_UUID],
    });

    bleDevice.addEventListener("gattserverdisconnected", onDisconnected);

    appendLog("Connecting to BLE...");
    const server = await bleDevice.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    rxCharacteristic = await service.getCharacteristic(RX_UUID);
    txCharacteristic = await service.getCharacteristic(TX_UUID);

    await txCharacteristic.startNotifications();
    txCharacteristic.addEventListener(
      "characteristicvaluechanged",
      handleBLENotifications
    );

    connectionMode = "ble";
    onConnected("BLE ONLINE");
    sendCommand("m"); // Request initial data
  } catch (e) {
    appendLog("BLE Error: " + e.message);
  }
}

function onConnected(text) {
  statusDot.classList.add("connected");
  statusText.innerText = text;
  disconnectBtn.classList.add("active");

  // Mostrar elementos que requieren conexión y ocultar los de estado offline
  connectOnlyElements.forEach((el) => el.classList.remove("hidden"));
  offlineOnlyElements.forEach((el) => el.classList.add("hidden"));

  setUIEnabled(true);
  appendLog("Connected via " + connectionMode.toUpperCase());
}

function onDisconnected() {
  appendLog("Disconnected.");
  statusDot.classList.remove("connected");
  statusText.innerText = "System Offline";
  disconnectBtn.classList.remove("active");

  // Ocultar elementos de conexión y mostrar los de estado offline
  connectOnlyElements.forEach((el) => el.classList.add("hidden"));
  offlineOnlyElements.forEach((el) => el.classList.remove("hidden"));

  setUIEnabled(false);
}

function setUIEnabled(enabled) {
  calibBtn.disabled = !enabled;
  diagBtn.disabled = !enabled;
  monitorBtn.disabled = !enabled;
  advancedToggle.disabled = !enabled;
  resetBtn.disabled = !enabled;
}

// --- Data Receivers ---

async function readLoopSerial() {
  const textDecoder = new TextDecoderStream();
  const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
  reader = textDecoder.readable.getReader(); // Usar variable global
  let buffer = "";

  try {
    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      processData(value, buffer, (newBuf) => (buffer = newBuf));
    }
  } catch (e) {
    console.error("Read Loop Error:", e);
    appendLog("Connection lost.");
  } finally {
    reader.releaseLock();
    // Si sigue marcado como conectado (ej: cable desconectado), forzar desconexión UI
    if (connectionMode === "serial") {
      port = null;
      connectionMode = null;
      onDisconnected();
    }
  }
}

let bleBuffer = "";
function handleBLENotifications(event) {
  const value = new TextDecoder().decode(event.target.value);
  processData(value, bleBuffer, (newBuf) => (bleBuffer = newBuf));
}

function processData(chunk, bufferRef, setBuffer) {
  let fullBuffer = bufferRef + chunk;
  let lines = fullBuffer.split("\n");
  setBuffer(lines.pop());

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

// --- UI Updates ---

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

    // Si recibimos datos de calibración, ocultamos el modal (proceso finalizado)
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

// --- Outgoing Commands ---

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
  if (connectionMode === "serial" && port) {
    const writer = port.writable.getWriter();
    await writer.write(new TextEncoder().encode(cmd + "\n"));
    writer.releaseLock();
  } else if (connectionMode === "ble" && rxCharacteristic) {
    await rxCharacteristic.writeValue(new TextEncoder().encode(cmd + "\n"));
  }
}

function appendLog(msg) {
  const div = document.createElement("div");
  div.innerText = `> ${msg}`;
  log.appendChild(div);
  log.scrollTop = log.scrollHeight;

  // Detectar instrucciones para el modal
  if (calibModal.classList.contains("active")) {
    if (msg.includes("No presiones") || msg.includes("SUELTA")) {
      modalStepText.innerHTML =
        '<span style="color: var(--brake); font-size: 1.5rem; font-weight: 900;">SUELTA EL PEDAL</span><br>y no lo toques';
    } else if (msg.includes("completamente") || msg.includes("PISA")) {
      modalStepText.innerHTML =
        '<span style="color: var(--primary); font-size: 1.5rem; font-weight: 900;">PISE A FONDO</span><br>y mantenga la presión';
    } else if (msg.includes("muestras")) {
      modalStepText.innerHTML =
        '<span style="color: #ffca28; font-size: 1.2rem; font-weight: 900;">TOMANDO MUESTRAS...</span><br>No sueltes todavía';
    }
  }
}

// --- Event Listeners ---

connectSerialBtn.addEventListener("click", () => {
  if (connectionMode === "serial") return;
  connectSerial();
});

connectBleBtn.addEventListener("click", () => {
  if (connectionMode === "ble") return;
  connectBLE();
});

disconnectBtn.addEventListener("click", async () => {
  if (connectionMode === "serial" && port) {
    if (reader) {
      // Intentar cancelar el reader de forma segura
      try {
        await reader.cancel();
        reader.releaseLock();
      } catch (e) {
        console.error("Error al cerrar reader:", e);
      }
      reader = null;
    }
    try {
      await port.close();
    } catch (e) {
      console.error("Error al cerrar puerto:", e);
    }
    port = null;
    connectionMode = null;
    onDisconnected();
  } else if (connectionMode === "ble" && bleDevice) {
    bleDevice.gatt.disconnect();
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
