const el = (id) => document.getElementById(id);

const ipInput = el("ip");
const hzInput = el("hz");
const btnConnect = el("connect");
const btnDisconnect = el("disconnect");
const statusText = el("statusText");
const dot = el("dot");
const errorBox = el("error");
const distText = el("distText");

const btnUp = el("btnUp");
const btnDown = el("btnDown");
const btnLeft = el("btnLeft");
const btnRight = el("btnRight");
const btnUpLeft = el("btnUpLeft");
const btnUpRight = el("btnUpRight");
const btnDownLeft = el("btnDownLeft");
const btnDownRight = el("btnDownRight");
const btnStop = el("btnStop");

const btnEngine = el("btnEngine");
const btnHorn = el("btnHorn");
const btnLights = el("btnLights");
const stateEngine = el("stateEngine");
const stateHorn = el("stateHorn");
const stateLights = el("stateLights");

let connected = false;
let deviceBase = "";
let startTimeMs = 0;
let timer = null;
let pointsLimit = 600;

let sessionId = 0;
let queueTail = Promise.resolve();

const series = {
  a_set: [],
  b_set: [],
  a_meas: [],
  b_meas: [],
  avg_set: [],
  avg_meas: [],
};

const toggles = {
  engine: false,
  horn: false,
  lights: false,
};

function normalizeBaseUrl(raw) {
  let v = (raw || "").trim();
  if (!v) return "";
  if (!/^https?:\/\//i.test(v)) v = "http://" + v;
  try {
    const u = new URL(v);
    return `${u.protocol}//${u.host}`;
  } catch {
    return v.replace(/\/+$/, "");
  }
}

function hzToMs(hz) {
  const h = Number(hz);
  if (!isFinite(h) || h <= 0) return 1000;
  return Math.max(50, Math.round(1000 / h));
}

function setStatus(kind, text) {
  statusText.textContent = text;
  dot.classList.remove("ok", "bad");
  if (kind === "ok") dot.classList.add("ok");
  if (kind === "bad") dot.classList.add("bad");
}

function setError(msg) {
  if (!msg) {
    errorBox.classList.remove("show");
    errorBox.textContent = "";
    return;
  }
  errorBox.textContent = msg;
  errorBox.classList.add("show");
}

function parseDist(v) {
  if (v === null || v === undefined) return null;

  if (typeof v === "number") return isFinite(v) ? v : null;

  if (typeof v === "string") {
    const s = v.trim().replace(",", ".");
    const m = s.match(/-?\d+(\.\d+)?/);
    if (!m) return null;
    const n = parseFloat(m[0]);
    return isFinite(n) ? n : null;
  }

  return null;
}

function setDistance(v) {
  const node = el("distText");
  if (!node) return;

  const n = parseDist(v);
  node.textContent = n === null ? "-" : String(n);
}


function setControlsEnabled(enabled) {
  btnUp.disabled = !enabled;
  btnDown.disabled = !enabled;
  btnLeft.disabled = !enabled;
  btnRight.disabled = !enabled;

  btnUpLeft.disabled = !enabled;
  btnUpRight.disabled = !enabled;
  btnDownLeft.disabled = !enabled;
  btnDownRight.disabled = !enabled;

  btnStop.disabled = !enabled;

  btnEngine.disabled = !enabled;
  btnHorn.disabled = !enabled;
  btnLights.disabled = !enabled;
}

function setToggleUI(key, on) {
  if (key === "engine") {
    toggles.engine = !!on;
    btnEngine.classList.toggle("active", toggles.engine);
    stateEngine.textContent = toggles.engine ? "ON" : "OFF";
  }
  if (key === "horn") {
    toggles.horn = !!on;
    btnHorn.classList.toggle("active", toggles.horn);
    stateHorn.textContent = toggles.horn ? "ON" : "OFF";
  }
  if (key === "lights") {
    toggles.lights = !!on;
    btnLights.classList.toggle("active", toggles.lights);
    stateLights.textContent = toggles.lights ? "ON" : "OFF";
  }
}

function trimSeries() {
  for (const k of Object.keys(series)) {
    if (series[k].length > pointsLimit) {
      series[k].splice(0, series[k].length - pointsLimit);
    }
  }
}

function addPoint(payload) {
  const now = Date.now();
  const t = (now - startTimeMs) / 1000;

  const a_set = Number(payload?.a_set ?? 0);
  const b_set = Number(payload?.b_set ?? 0);
  const a_meas = Number(payload?.a_meas ?? 0);
  const b_meas = Number(payload?.b_meas ?? 0);

  const avg_set = (a_set + b_set) / 2;
  const avg_meas = (a_meas + b_meas) / 2;

  series.a_set.push({ x: t, y: a_set });
  series.b_set.push({ x: t, y: b_set });
  series.a_meas.push({ x: t, y: a_meas });
  series.b_meas.push({ x: t, y: b_meas });
  series.avg_set.push({ x: t, y: avg_set });
  series.avg_meas.push({ x: t, y: avg_meas });

  trimSeries();

  if (payload && typeof payload === "object") {
    if ("dist" in payload) setDistance(payload.dist);

    if ("engine" in payload) setToggleUI("engine", Number(payload.engine) === 1);
    if ("horn" in payload) setToggleUI("horn", Number(payload.horn) === 1);
    if ("lights" in payload) setToggleUI("lights", Number(payload.lights) === 1);
    if (!("lights" in payload) && ("ind" in payload)) setToggleUI("lights", Number(payload.ind) === 1);
  }
}

function makeChart(canvasId, datasets) {
  const canvas = el(canvasId);
  if (!canvas) throw new Error(`Brak elementu <canvas id="${canvasId}"> w HTML`);
  const ctx = canvas.getContext("2d");
  if (!ctx) throw new Error(`Nie mogę pobrać 2D context dla canvas "${canvasId}"`);

  return new Chart(ctx, {
    type: "line",
    data: { datasets },
    options: {
      responsive: true,
      animation: false,
      maintainAspectRatio: false,
      parsing: false,
      normalized: true,
      plugins: {
        legend: { display: true },
        title: { display: false },
        tooltip: { callbacks: { label: (c) => `${c.dataset.label}: ${c.parsed.y} RPM` } }
      },
      scales: {
        x: { type: "linear", title: { display: true, text: "Czas [s]" }, ticks: { maxTicksLimit: 10 } },
        y: { title: { display: true, text: "RPM" }, ticks: { maxTicksLimit: 8 } }
      },
      elements: {
        point: { radius: 0 },
        line: { borderWidth: 2, tension: 0.15 }
      }
    }
  });
}

const chartA = makeChart("chartA", [
  { label: "A SET", data: series.a_set, borderDash: [6, 6] },
  { label: "A MEAS", data: series.a_meas },
]);

const chartB = makeChart("chartB", [
  { label: "B SET", data: series.b_set, borderDash: [6, 6] },
  { label: "B MEAS", data: series.b_meas },
]);

const chartAvg = makeChart("chartAvg", [
  { label: "AVG SET", data: series.avg_set, borderDash: [6, 6] },
  { label: "AVG MEAS", data: series.avg_meas },
]);

function updateCharts() {
  chartA.update("none");
  chartB.update("none");
  chartAvg.update("none");
}

async function fetchJson(url, timeoutMs = 0) {
  const controller = new AbortController();
  let to = null;
  if (timeoutMs && timeoutMs > 0) to = setTimeout(() => controller.abort(), timeoutMs);

  try {
    const res = await fetch(url, { method: "GET", signal: controller.signal, cache: "no-store", mode: "cors" });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    return await res.json();
  } finally {
    if (to) clearTimeout(to);
  }
}

function enqueue(task) {
  const mySession = sessionId;
  const run = queueTail.then(async () => {
    if (mySession !== sessionId) return;
    return task();
  });
  queueTail = run.catch(() => {});
  return run;
}

function apiUrl(path) {
  return `http://127.0.0.1:8787${path}?ip=${encodeURIComponent(deviceBase)}`;
}

function resetData() {
  for (const k of Object.keys(series)) series[k].length = 0;
  updateCharts();
}

async function sendMotor(code, force = false) {
  if ((!connected && !force) || !deviceBase) return;
  return enqueue(() => fetchJson(apiUrl(`/motor/${code}`), 4000));
}

async function sendToggle(path, key) {
  if (!connected || !deviceBase) return;
  try {
    const payload = await enqueue(() => fetchJson(apiUrl(path), 4000));
    if (payload && typeof payload === "object") {
      if (key === "engine" && "engine" in payload) setToggleUI("engine", Number(payload.engine) === 1);
      else if (key === "horn" && "horn" in payload) setToggleUI("horn", Number(payload.horn) === 1);
      else if (key === "lights" && ("lights" in payload || "ind" in payload)) {
        if ("lights" in payload) setToggleUI("lights", Number(payload.lights) === 1);
        else setToggleUI("lights", Number(payload.ind) === 1);
      } else {
        setToggleUI(key, !toggles[key]);
      }
      if ("dist" in payload) setDistance(payload.dist);
    } else {
      setToggleUI(key, !toggles[key]);
    }
  } catch (e) {
    setError(`Nie udało się wykonać ${path}: ${e?.message ?? e}`);
  }
}

let activePointerId = null;

function bindHoldButton(btn, code) {
  const onDown = (e) => {
    if (!connected) return;
    if (activePointerId !== null) return;

    activePointerId = e.pointerId ?? "mouse";

    if (btn.setPointerCapture && e.pointerId !== undefined) {
      try { btn.setPointerCapture(e.pointerId); } catch {}
    }

    setError("");
    sendMotor(code).catch((err) => setError(`motor/${code}: ${err?.message ?? err}`));
  };

  const onUp = (e) => {
    const pid = e.pointerId ?? "mouse";
    if (activePointerId !== pid) return;

    activePointerId = null;

    setError("");
    sendMotor(1).catch((err) => setError(`motor/1: ${err?.message ?? err}`));
  };

  btn.addEventListener("pointerdown", onDown);
  btn.addEventListener("pointerup", onUp);
  btn.addEventListener("pointercancel", onUp);
}

bindHoldButton(btnUp, 2);
bindHoldButton(btnUpRight, 3);
bindHoldButton(btnRight, 4);
bindHoldButton(btnDownRight, 5);
bindHoldButton(btnDown, 6);
bindHoldButton(btnDownLeft, 7);
bindHoldButton(btnLeft, 8);
bindHoldButton(btnUpLeft, 9);

btnStop.addEventListener("click", () => {
  if (!connected) return;
  setError("");
  sendMotor(1).catch((err) => setError(`motor/1: ${err?.message ?? err}`));
});

btnEngine.addEventListener("click", () => sendToggle("/engine", "engine"));
btnHorn.addEventListener("click", () => sendToggle("/horn", "horn"));
btnLights.addEventListener("click", () => sendToggle("/lights", "lights"));

async function pollLoop(intervalMs, mySession) {
  if (!connected) return;
  if (mySession !== sessionId) return;

  const t0 = performance.now();
  try {
    const telemetryTimeout = Math.max(5000, intervalMs * 5);
    const payload = await enqueue(() => fetchJson(apiUrl("/telemetry"), telemetryTimeout));
    if (mySession !== sessionId || !payload) return;

    if (payload && typeof payload === "object" && "dist" in payload) {
      setDistance(payload.dist);
    }


    if (Number(payload?.ok) !== 1) setStatus("bad", "Połączono, ale urządzenie zwraca ok != 1");
    else setStatus("ok", "Połączono i odbieram telemetrię");

    setError("");
    addPoint(payload);
    updateCharts();
  } catch (e) {
    if (mySession !== sessionId) return;
    setStatus("bad", "Błąd pobierania /telemetry");
    setError(`Nie udało się pobrać telemetrii: ${e?.message ?? e}`);
  }

  const spent = performance.now() - t0;
  const delay = Math.max(0, intervalMs - Math.round(spent));
  timer = setTimeout(() => pollLoop(intervalMs, mySession), delay);
}

async function connect() {
  setError("");

  sessionId++;
  const mySession = sessionId;

  deviceBase = normalizeBaseUrl(ipInput.value);
  if (!deviceBase) {
    setError("Podaj adres IP/host.");
    return;
  }

  const intervalMs = hzToMs(hzInput.value);

  btnConnect.disabled = true;
  btnDisconnect.disabled = false;
  ipInput.disabled = true;
  hzInput.disabled = true;
  setControlsEnabled(false);

  resetData();
  setDistance("-");
  setStatus("", "Łączę…");

  try {
    const initPayload = await enqueue(() => fetchJson(apiUrl("/init"), 20000));
    if (mySession !== sessionId) return;

    if (initPayload && typeof initPayload === "object") {
      if ("engine" in initPayload) setToggleUI("engine", Number(initPayload.engine) === 1);
      if ("horn" in initPayload) setToggleUI("horn", Number(initPayload.horn) === 1);
      if ("lights" in initPayload) setToggleUI("lights", Number(initPayload.lights) === 1);
      if (!("lights" in initPayload) && ("ind" in initPayload)) setToggleUI("lights", Number(initPayload.ind) === 1);
      if ("dist" in initPayload) setDistance(initPayload.dist);
    }

    connected = true;
    startTimeMs = Date.now();
    setStatus("ok", "Połączono (init OK). Start próbkowania…");
    setControlsEnabled(true);

    pollLoop(intervalMs, mySession);
  } catch (e) {
    if (mySession !== sessionId) return;

    connected = false;
    setStatus("bad", "Nie udało się połączyć");
    setError(`Nie udało się wykonać GET /init: ${e?.message ?? e}`);

    btnConnect.disabled = false;
    btnDisconnect.disabled = true;
    ipInput.disabled = false;
    hzInput.disabled = false;
    setControlsEnabled(false);
  }
}

function disconnect() {
  sessionId++;

  sendMotor(1, true).catch(() => {});
  activePointerId = null;

  connected = false;
  if (timer) clearTimeout(timer);
  timer = null;

  btnConnect.disabled = false;
  btnDisconnect.disabled = true;
  ipInput.disabled = false;
  hzInput.disabled = false;
  setControlsEnabled(false);

  setStatus("", "Rozłączono");
  setError("");
  setDistance("-");
}

btnConnect.addEventListener("click", connect);
btnDisconnect.addEventListener("click", disconnect);
