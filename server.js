const express = require("express");
const path = require("path");

const app = express();
app.use(express.json());

/* ===== ДАННЫЕ ОТ ESP ===== */
let sensorData = {
  temperature: 0,
  pressure: 0,
  humidity: 0,
  light: 0,
  isRaining: false
};

/* ===== HEALTH CHECK (Railway) ===== */
app.get("/health", (req, res) => {
  res.status(200).send("OK");
});

/* ===== ESP ОТПРАВЛЯЕТ ДАННЫЕ ===== */
app.post("/data", (req, res) => {
  sensorData = req.body;
  console.log("📡 Data from ESP:", sensorData);
  res.json({ status: "ok" });
});

/* ===== САЙТ ЗАПРАШИВАЕТ ДАННЫЕ ===== */
app.get("/data", (req, res) => {
  res.json(sensorData);
});

/* ===== ГЛАВНАЯ СТРАНИЦА ===== */
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

/* ===== ЗАПУСК СЕРВЕРА ===== */
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log("🚀 Server running on port", PORT);
});
