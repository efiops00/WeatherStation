const express = require("express");
const fs = require("fs"); // Для персистента
const path = require("path");

const app = express();
const PORT = process.env.PORT || 8080;
const DATA_FILE = path.join(__dirname, "data.json");

// ─────────────────────────────
// CORS для GitHub Pages
// ─────────────────────────────
app.use((req, res, next) => {
  res.setHeader("Access-Control-Allow-Origin", "*");
  res.setHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  res.setHeader("Access-Control-Allow-Headers", "Content-Type");
  if (req.method === "OPTIONS") {
    return res.sendStatus(204);
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
  return {
    temperature: 0,
    pressure: 0,
    humidity: 0,
    light: 0,
    isRaining: false,
    updatedAt: null
  };
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
// Middleware
// ─────────────────────────────
app.use(express.json());

// ─────────────────────────────
// Главная страница (можно не использовать,
// если фронтенд на GitHub Pages)
// ─────────────────────────────
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

// ─────────────────────────────
// Получение данных (читаем из файла)
// ─────────────────────────────
app.get("/data", (req, res) => {
  const data = readData();
  console.log("📡 GET data served:", data); // Лог для Railway
  res.setHeader("Cache-Control", "no-store, no-cache, must-revalidate, private");
  res.setHeader("Pragma", "no-cache");
  res.setHeader("Expires", "0");
  res.json(data);
});

// ─────────────────────────────
// Приём данных от ESP (обновляем файл)
// ─────────────────────────────
app.post("/data", (req, res) => {
  const body = req.body;
  console.log("📥 Raw POST body:", body); // Debug raw
  if (!body || Object.keys(body).length === 0) {
    console.log("❌ Empty POST body");
    return res.status(400).json({ status: "error", message: "Empty body" });
  }

  const newData = {
    temperature: Number(body.temperature) || 0,
    pressure: Number(body.pressure) || 0,
    humidity: Number(body.humidity) || 0,
    light: Number(body.light) || 0,
    isRaining: Boolean(body.isRaining),
    updatedAt: new Date().toISOString()
  };

  console.log("📡 DATA RECEIVED & UPDATED:", newData);
  writeData(newData);
  res.json({ status: "ok" });
});

// ─────────────────────────────
// Тест: сброс данных
// ─────────────────────────────
app.post("/reset", (req, res) => {
  writeData({
    temperature: 0,
    pressure: 0,
    humidity: 0,
    light: 0,
    isRaining: false,
    updatedAt: null
  });
  console.log("🔄 Data reset");
  res.json({ status: "reset ok" });
});

// ─────────────────────────────
// Запуск
// ─────────────────────────────
app.listen(PORT, () => {
  console.log(`🚀 Server on port ${PORT}`);
  console.log("📂 Data file:", DATA_FILE);
});
