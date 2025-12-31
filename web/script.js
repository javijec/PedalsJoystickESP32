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
  connectBtn.innerText = "DISCONNECT";
  setUIEnabled(true);
  appendLog("Connected via " + connectionMode.toUpperCase());
}

function onDisconnected() {
  appendLog("Disconnected.");
  window.location.reload();
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
  port.readable.pipeTo(textDecoder.writable);
  const reader = textDecoder.readable.getReader();
  let buffer = "";

  try {
    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      processData(value, buffer, (newBuf) => (buffer = newBuf));
    }
  } catch (e) {
    appendLog("Read Error: " + e.message);
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

  if (data.cal) {
    gMin.innerText = data.cal.gmin;
    gMax.innerText = data.cal.gmax;
    bMax.innerText = data.cal.bmax.toFixed(0);
    if (data.cal.bmax > 0)
      bScale.innerText = (16384 / data.cal.bmax).toFixed(4);
    cMin.innerText = data.cal.cmin;
    cMax.innerText = data.cal.cmax;
  }
}

// --- Outgoing Commands ---

async function calibrateIndividual(pedal) {
  let command = "";
  if (pedal === "g") command = "g";
  else if (pedal === "b") command = "b";
  else if (pedal === "c") command = "e";

  if (command) {
    await sendCommand(command);
    appendLog(
      `Iniciando calibración individual para ${pedal.toUpperCase()}...`
    );
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
}

// --- Event Listeners ---

connectBtn.addEventListener("click", async () => {
  if (connectionMode) {
    onDisconnected();
  } else {
    // Show simple selection or try Serial first as default
    const choice = confirm("Press OK for USB Serial, Cancel for Bluetooth");
    if (choice) connectSerial();
    else connectBLE();
  }
});

calibBtn.addEventListener("click", () => {
  sendCommand("c");
  appendLog("Starting calibration...");
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
