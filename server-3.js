const express = require("express");
const fs = require("fs"); 
const path = require("path");

const app = express();
const PORT = process.env.PORT || 8080;
const DATA_FILE = path.join(__dirname, "data.json");

// ─────────────────────────────
// Middleware
// ─────────────────────────────
app.use(express.json());

// Добавляем CORS заголовки вручную, чтобы Railway Edge не блокировал POST
app.use((req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, PATCH, DELETE");
  res.header("Access-Control-Allow-Headers", "X-Requested-With,content-type");
  
  // Ответ на preflight-запросы
  if (req.method === 'OPTIONS') {
    return res.sendStatus(200);
  }
  next();
});

// ─────────────────────────────
// Функция чтения/записи данных
// ─────────────────────────────
function readData() {
  if (fs.existsSync(DATA_FILE)) {
    try {
      return JSON.parse(fs.readFileSync(DATA_FILE, "utf8"));
    } catch (e) {
      console.error("❌ Read data error:", e);
    }
  }
  return { temperature: 0, pressure: 0, humidity: 0, light: 0, isRaining: false, updatedAt: null };
}

function writeData(data) {
  try {
    fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2));
    console.log("💾 Data saved to file");
  } catch (e) {
    console.error("❌ Write data error:", e);
  }
}

// ─────────────────────────────
// Маршруты
// ─────────────────────────────
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

app.get("/data", (req, res) => {
  const data = readData();
  res.setHeader("Cache-Control", "no-store, no-cache, must-revalidate, private");
  res.json(data);
});

// Главный обработчик POST от Arduino
app.post("/data", (req, res) => {
  const body = req.body;
  console.log("📥 Raw POST body received:", body); 
  
  if (!body || Object.keys(body).length === 0) {
    console.log("❌ Empty POST body");
    return res.status(400).json({ status: "error", message: "Empty body" });
  }

  const newData = {
    temperature: Number(body.temperature) || 0,
    pressure: Number(body.pressure) || 0,
    humidity: Number(body.humidity) || 0,
    light: Number(body.light) || 0,
    isRaining: body.isRaining === true || body.isRaining === "true",
    updatedAt: new Date().toISOString(),
  };

  console.log("📡 DATA RECEIVED & UPDATED:", newData);
  writeData(newData);
  res.status(200).json({ status: "ok" }); // Явно указываем 200 OK
});

// ─────────────────────────────
// Запуск
// ─────────────────────────────
// ВАЖНО: '0.0.0.0' обязателен для Railway, иначе контейнер не будет доступен извне!
app.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Server on port ${PORT}`);
  console.log("📂 Data file:", DATA_FILE);
});
