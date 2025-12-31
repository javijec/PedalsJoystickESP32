let port;
let reader;
let inputDone;
let inputStream;

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

// Elementos de calibración
const gMin = document.getElementById("g-min");
const gMax = document.getElementById("g-max");
const bMax = document.getElementById("b-max");
const bScale = document.getElementById("b-scale");
const cMin = document.getElementById("c-min");
const cMax = document.getElementById("c-max");

// Deazones UI elements
const gDZL = document.getElementById("g-dzl");
const gDZH = document.getElementById("g-dzh");
const bDZL = document.getElementById("b-dzl");
const bDZH = document.getElementById("b-dzh");
const cDZL = document.getElementById("c-dzl");
const cDZH = document.getElementById("c-dzh");

async function connect() {
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });

    statusDot.classList.add("connected");
    statusText.innerText = "SYSTEM ONLINE";
    connectBtn.innerText = "TERMINATE CONNECTION";
    calibBtn.disabled = false;
    diagBtn.disabled = false;
    monitorBtn.disabled = false;
    advancedToggle.disabled = false;
    resetBtn.disabled = false;

    appendLog("Conectado exitosamente");

    readLoop();
  } catch (e) {
    let errorMsg = e.message;
    if (e.name === "NetworkError") {
      errorMsg =
        "El puerto está siendo usado por otra aplicación (ej: Monitor Serie de Arduino IDE). Ciérrala e intenta de nuevo.";
    } else if (e.name === "SecurityError") {
      errorMsg = "Permiso denegado por el navegador.";
    }
    appendLog("Error de conexión: " + errorMsg);
    console.error(e);
  }
}

async function readLoop() {
  const textDecoder = new TextDecoderStream();
  inputDone = port.readable.pipeTo(textDecoder.writable);
  inputStream = textDecoder.readable;
  reader = inputStream.getReader();

  let buffer = "";

  try {
    while (true) {
      const { value, done } = await reader.read();
      if (done) break;

      buffer += value;

      // Procesar líneas completas
      let lines = buffer.split("\n");
      buffer = lines.pop(); // Mantener la última línea incompleta

      for (let line of lines) {
        line = line.trim();
        if (line.startsWith("{")) {
          try {
            const data = JSON.parse(line);
            updateUI(data);
          } catch (e) {
            // Ignorar errores de parseo parcial
          }
        } else if (line.length > 0) {
          appendLog(line);
        }
      }
    }
  } catch (e) {
    appendLog("Error de lectura: " + e.message);
  } finally {
    reader.releaseLock();
  }
}

function updateUI(data) {
  // Escalar valores (Arduino envía 0-4095 para gas/clutch, 0-16384 para brake)
  const gasPct = (data.g / 4095) * 100;
  const brakePct = (data.b / 16384) * 100;
  const clutchPct = (data.c / 4095) * 100;

  gasBar.style.width = `${gasPct}%`;
  gasVal.innerText = `${Math.round(gasPct)}%`;

  brakeBar.style.width = `${brakePct}%`;
  brakeVal.innerText = `${Math.round(brakePct)}%`;

  clutchBar.style.width = `${clutchPct}%`;
  clutchVal.innerText = `${Math.round(clutchPct)}%`;

  // Datos de calibración
  if (data.cal) {
    gMin.innerText = data.cal.gmin;
    gMax.innerText = data.cal.gmax;
    bMax.innerText = data.cal.bmax.toFixed(0);
    if (data.cal.bmax > 0) {
      bScale.innerText = (16384 / data.cal.bmax).toFixed(4);
    } else {
      bScale.innerText = "inf";
    }
    cMin.innerText = data.cal.cmin;
    cMax.innerText = data.cal.cmax;

    // Deadzones (solo actualizar si no tienen el foco para no molestar al usuario)
    if (document.activeElement !== gDZL) gDZL.value = data.cal.gdzl;
    if (document.activeElement !== gDZH) gDZH.value = data.cal.gdzh;
    if (document.activeElement !== bDZL) bDZL.value = data.cal.bdzl;
    if (document.activeElement !== bDZH) bDZH.value = data.cal.bdzh;
    if (document.activeElement !== cDZL) cDZL.value = data.cal.cdzl;
    if (document.activeElement !== cDZH) cDZH.value = data.cal.cdzh;
  }
}

function appendLog(msg) {
  const div = document.createElement("div");
  div.innerText = `> ${msg}`;
  log.appendChild(div);
  log.scrollTop = log.scrollHeight;
}

connectBtn.addEventListener("click", () => {
  if (port) {
    appendLog("Conexión finalizada.");
    window.location.reload();
  } else {
    connect();
  }
});

calibBtn.addEventListener("click", () => {
  const writer = port.writable.getWriter();
  writer.write(new TextEncoder().encode("c"));
  writer.releaseLock();
  appendLog("Iniciando calibración... Sigue las instrucciones en el panel.");
});

diagBtn.addEventListener("click", () => {
  const writer = port.writable.getWriter();
  writer.write(new TextEncoder().encode("d"));
  writer.releaseLock();
  appendLog("Solicitando diagnóstico raw...");
});

monitorBtn.addEventListener("click", () => {
  const writer = port.writable.getWriter();
  writer.write(new TextEncoder().encode("m"));
  writer.releaseLock();
  appendLog("Toggled monitoreo continuo.");
});

async function saveDZ(pedal) {
  if (!port) return;

  let low, high;
  if (pedal === "g") {
    low = parseInt(gDZL.value);
    high = parseInt(gDZH.value);
  } else if (pedal === "b") {
    low = parseInt(bDZL.value);
    high = parseInt(bDZH.value);
  } else if (pedal === "c") {
    low = parseInt(cDZL.value);
    high = parseInt(cDZH.value);
  }

  const cmd = {
    cmd: "setDZ",
    p: pedal,
    low: low,
    high: high,
  };

  const jsonString = JSON.stringify(cmd) + "\n";
  const writer = port.writable.getWriter();
  await writer.write(new TextEncoder().encode(jsonString));
  writer.releaseLock();

  appendLog(
    `Enviando nueva zona muerta para ${pedal}: Low=${low}%, High=${high}%`
  );
}

advancedToggle.addEventListener("click", () => {
  advancedSection.classList.toggle("visible");
  advancedToggle.innerText = advancedSection.classList.contains("visible")
    ? "HIDE ADVANCED"
    : "ADVANCED TOOLS";
});

resetBtn.addEventListener("click", async () => {
  if (!port) return;
  if (
    !confirm(
      "¿Estás seguro de que quieres resetear toda la configuración a los valores por defecto?"
    )
  )
    return;

  const writer = port.writable.getWriter();
  await writer.write(new TextEncoder().encode("r"));
  writer.releaseLock();
  appendLog("Comando de reseteo enviado.");
});
