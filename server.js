const express = require("express");
const path = require("path");

const app = express();
app.use(express.json());

// ===== ХРАНЕНИЕ ДАННЫХ ОТ ESP =====
let sensorData = {
  temperature: 0,
  pressure: 0,
  humidity: 0,
  light: 0,
  isRaining: false,
  tempValid: false,
  pressValid: false,
  humValid: false
};

// ===== ESP8266 ОТПРАВЛЯЕТ ДАННЫЕ =====
app.post("/data", (req, res) => {
  sensorData = req.body;
  console.log("📡 Data from ESP:", sensorData);
  res.json({ status: "ok" });
});

// ===== САЙТ ЧИТАЕТ ДАННЫЕ =====
app.get("/data", (req, res) => {
  res.json(sensorData);
});

// ===== ГЛАВНАЯ СТРАНИЦА =====
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

// ===== ЗАПУСК СЕРВЕРА (ВАЖНО ДЛЯ RAILWAY) =====
const PORT = process.env.PORT || 3000;

app.listen(PORT, () => {
  console.log("🚀 Server running on port", PORT);
});
