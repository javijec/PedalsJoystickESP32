// --- Global Variables ---
let port; // Serial Port
let reader; // Serial Reader
let connectionMode = null; // 'serial' or 'ble'

// BLE Globals
let bleDevice;
let bleServer;
let rxCharacteristic;
let txCharacteristic;

const SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // Mobile writes
const TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // Mobile receives

// UI Elements
const connectSerialBtn = document.getElementById("connectSerialBtn");
const connectBleBtn = document.getElementById("connectBleBtn");
const connectBtn = document.getElementById("connectBtn"); // Botón genérico si existe
const calibBtn = document.getElementById("calibBtn");
const diagBtn = document.getElementById("diagBtn");
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
const graphSection = document.getElementById("graphSection");

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

// --- Signal Monitor Class ---
class SignalMonitor {
  constructor(canvasId) {
    this.canvas = document.getElementById(canvasId);
    this.ctx = this.canvas.getContext("2d");
    this.width = this.canvas.width = this.canvas.offsetWidth;
    this.height = this.canvas.height = 150;

    // Configuración
    this.maxPoints = 200; // Ancho del buffer
    this.data = { g: [], b: [], c: [] };

    // Inicializar con ceros
    for (let i = 0; i < this.maxPoints; i++) {
      this.data.g.push(0);
      this.data.b.push(0);
      this.data.c.push(0);
    }

    this.running = false;
    this.draw = this.draw.bind(this);

    // Resize handler
    window.addEventListener("resize", () => {
      if (this.canvas) {
        this.width = this.canvas.width = this.canvas.offsetWidth;
      }
    });
  }

  push(g, b, c) {
    if (!this.running) return;

    // Normalizar a 0-1 usando el rango de salida del Joystick
    // Gas/Clutch (4095), Brake (16384)
    this.data.g.push(Math.min(1, Math.max(0, g / 4095)));
    this.data.b.push(Math.min(1, Math.max(0, b / 16384)));
    this.data.c.push(Math.min(1, Math.max(0, c / 4095)));

    if (this.data.g.length > this.maxPoints) this.data.g.shift();
    if (this.data.b.length > this.maxPoints) this.data.b.shift();
    if (this.data.c.length > this.maxPoints) this.data.c.shift();
  }

  start() {
    this.running = true;
    // Forzar actualización de dimensiones al iniciar (ahora que es visible)
    if (this.canvas) {
      const rect = this.canvas.getBoundingClientRect();
      if (rect.width > 0) {
        this.width = this.canvas.width = rect.width;
        this.height = this.canvas.height = 150;
      }
    }
    requestAnimationFrame(this.draw);
  }

  stop() {
    this.running = false;
  }

  clear() {
    if (this.ctx) {
      this.ctx.clearRect(0, 0, this.width, this.height);
    }
  }

  draw() {
    if (!this.running) return;

    // Auto-resize check per frame (simple y robusto para cambios de layout)
    if (this.canvas.width !== this.canvas.offsetWidth) {
      this.width = this.canvas.width = this.canvas.offsetWidth;
      this.height = this.canvas.height = 150;
    }

    this.clear();
    const step = this.width / this.maxPoints;

    // Helper para dibujar línea
    const drawLine = (dataArr, color) => {
      this.ctx.beginPath();
      this.ctx.strokeStyle = color;
      this.ctx.lineWidth = 2;
      this.ctx.lineJoin = "round";

      for (let i = 0; i < dataArr.length; i++) {
        // Invertir Y (canvas 0 es arriba)
        const y = this.height - dataArr[i] * this.height;
        const x = i * step;
        if (i === 0) this.ctx.moveTo(x, y);
        else this.ctx.lineTo(x, y);
      }
      this.ctx.stroke();
    };

    // Dibujar canales
    drawLine(
      this.data.g,
      getComputedStyle(document.documentElement)
        .getPropertyValue("--primary")
        .trim() || "#00e676"
    );
    drawLine(
      this.data.b,
      getComputedStyle(document.documentElement)
        .getPropertyValue("--brake")
        .trim() || "#ff1744"
    );
    drawLine(
      this.data.c,
      getComputedStyle(document.documentElement)
        .getPropertyValue("--clutch")
        .trim() || "#2979ff"
    );

    requestAnimationFrame(this.draw);
  }
}

let monitor;
if (document.getElementById("signalChart")) {
  monitor = new SignalMonitor("signalChart");
}

// --- Connection Logic (Serial) ---

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

async function readLoopSerial() {
  const textDecoder = new TextDecoderStream();
  const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
  reader = textDecoder.readable.getReader();
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
    if (reader) reader.releaseLock();
    // Auto-disconnect detect
    if (connectionMode === "serial") {
      port = null;
      connectionMode = null;
      onDisconnected();
    }
  }
}

// --- Connection Logic (BLE) ---

async function connectBLE() {
  try {
    appendLog("Scanning for PedalMaster BLE...");
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ name: "PedalMaster BLE" }],
      optionalServices: [SERVICE_UUID],
    });

    bleDevice.addEventListener("gattserverdisconnected", onDisconnected);

    appendLog("Connecting to GATT Server...");
    bleServer = await bleDevice.gatt.connect();

    appendLog("Retrieving Service...");
    const service = await bleServer.getPrimaryService(SERVICE_UUID);

    appendLog("Linking Characteristics...");
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
    onDisconnected();
  }
}

let bleBuffer = "";
function handleBLENotifications(event) {
  const value = new TextDecoder().decode(event.target.value);
  processData(value, bleBuffer, (newBuf) => (bleBuffer = newBuf));
}

// --- Common Data Processing ---

function processData(chunk, currentBuffer, setBuffer) {
  currentBuffer += chunk;
  let lines = currentBuffer.split("\n");
  setBuffer(lines.pop()); // Keep incomplete line

  for (let line of lines) {
    line = line.trim();
    if (line.startsWith("{")) {
      try {
        const data = JSON.parse(line);
        updateUI(data);
      } catch (e) {
        // Ignorar JSON corrupto ocasional
      }
    } else if (line.length > 0) {
      appendLog(line);
    }
  }
}

// --- UI Updates ---

function onConnected(text) {
  statusDot.classList.add("connected");
  statusText.innerText = text;
  disconnectBtn.classList.add("active");

  // Show UI
  connectOnlyElements.forEach((el) => el.classList.remove("hidden"));
  offlineOnlyElements.forEach((el) => el.classList.add("hidden"));
  if (graphSection) graphSection.classList.remove("hidden");

  // Start Monitor
  if (monitor) monitor.start();

  setUIEnabled(true);
  appendLog("Connected via " + connectionMode.toUpperCase());
}

function onDisconnected() {
  statusDot.classList.remove("connected");
  statusText.innerText = "SYSTEM OFFLINE";
  disconnectBtn.classList.remove("active");

  // Hide UI
  connectOnlyElements.forEach((el) => el.classList.add("hidden"));
  offlineOnlyElements.forEach((el) => el.classList.remove("hidden"));
  if (graphSection) graphSection.classList.add("hidden");

  // Stop Monitor
  if (monitor) {
    monitor.stop();
    monitor.clear();
  }

  setUIEnabled(false);
  appendLog("Disconnected.");
}

function setUIEnabled(enabled) {
  calibBtn.disabled = !enabled;
  diagBtn.disabled = !enabled;
  advancedToggle.disabled = !enabled;
  resetBtn.disabled = !enabled;
}

function updateUI(data) {
  // Update Bars
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

  // Update Raw Values & Monitor
  if (data.rg !== undefined) {
    if (rawG) rawG.innerText = data.rg;
    if (rawB) rawB.innerText = data.rb ? data.rb.toFixed(0) : 0;
    if (rawC) rawC.innerText = data.rc;

    // Push to Monitor (CALIBRATED Values 0-4095)
    // Usamos los valores finales 'data.g', 'data.b', 'data.c' para ver la respuesta real
    if (monitor && data.g !== undefined) {
      monitor.push(data.g, data.b, data.c);
    }
  }

  // Update Calibration Data
  if (data.cal) {
    gMin.innerText = data.cal.gmin;
    gMax.innerText = data.cal.gmax;
    bMax.innerText = data.cal.bmax.toFixed(0);
    if (data.cal.bmax > 0)
      bScale.innerText = (16384 / data.cal.bmax).toFixed(4);
    cMin.innerText = data.cal.cmin;
    cMax.innerText = data.cal.cmax;

    // Calibration received means calibration process finished
    hideCalibrationModal();
  }
}

// --- Outgoing Commands ---

async function sendCommand(cmd) {
  if (connectionMode === "serial" && port) {
    const writer = port.writable.getWriter();
    await writer.write(new TextEncoder().encode(cmd + "\n"));
    writer.releaseLock();
  } else if (connectionMode === "ble" && rxCharacteristic) {
    await rxCharacteristic.writeValue(new TextEncoder().encode(cmd + "\n"));
  }
}

// --- Modal & Calibration ---

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

function appendLog(msg) {
  const div = document.createElement("div");
  div.innerText = `> ${msg}`;
  log.appendChild(div);
  log.scrollTop = log.scrollHeight;

  // Modal Dynamic Instructions
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

if (connectSerialBtn) {
  connectSerialBtn.addEventListener("click", () => {
    if (connectionMode) return;
    connectSerial();
  });
}

if (connectBleBtn) {
  connectBleBtn.addEventListener("click", () => {
    if (connectionMode) return;
    connectBLE();
  });
}

if (connectBtn) {
  connectBtn.addEventListener("click", () => {
    // Generic handler, logic already covered above
  });
}

if (disconnectBtn) {
  disconnectBtn.addEventListener("click", async () => {
    if (connectionMode === "serial" && port) {
      if (reader) {
        try {
          await reader.cancel();
          reader.releaseLock();
        } catch (e) {
          console.error("Reader cancel error:", e);
        }
        reader = null;
      }
      try {
        await port.close();
      } catch (e) {
        console.error("Port close error:", e);
      }
      port = null;
      connectionMode = null;
      onDisconnected();
    } else if (connectionMode === "ble" && bleDevice) {
      if (bleDevice.gatt.connected) bleDevice.gatt.disconnect();
    }
  });
}

calibBtn.addEventListener("click", () => {
  showCalibrationModal("CALIBRACION COMPLETA");
  sendCommand("c"); // Auto-calibration trigger
  appendLog("Starting auto-calibration...");
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

advancedToggle.addEventListener("click", () => {
  advancedSection.classList.toggle("visible");
});
