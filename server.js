const express = require("express");
const path = require("path");

const app = express();

/**
 * Railway ВСЕГДА передаёт порт через process.env.PORT
 * НЕЛЬЗЯ хардкодить 8080
 */
const PORT = process.env.PORT || 8080;

/**
 * Хранилище последних данных от ESP
 */
let lastData = {
  temperature: 0,
  pressure: 0,
  humidity: 0,
  light: 0,
  isRaining: false,
};

/**
 * Разрешаем JSON
 */
app.use(express.json());

/**
 * Отдаём index.html по корню /
 */
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

/**
 * ESP шлёт данные сюда
 */
app.post("/data", (req, res) => {
  console.log("📡 Data received from ESP:", req.body);

  lastData = {
    temperature: req.body.temperature ?? 0,
    pressure: req.body.pressure ?? 0,
    humidity: req.body.humidity ?? 0,
    light: req.body.light ?? 0,
    isRaining: req.body.isRaining ?? false,
  };

  res.json({ status: "ok" });
});

/**
 * Браузер читает данные отсюда
 */
app.get("/data", (req, res) => {
  res.json(lastData);
});

/**
 * Запуск сервера
 */
app.listen(PORT, () => {
  console.log(`🚀 Server running on port ${PORT}`);
});
