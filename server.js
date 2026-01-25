const express = require("express");
const path = require("path");

const app = express();
app.use(express.json());

// 🔴 здесь храним последние данные от ESP
let lastData = {
  temperature: 0,
  pressure: 0,
  humidity: 0,
  light: 0,
  isRaining: false,
};

// 📥 ESP шлёт данные сюда
app.post("/data", (req, res) => {
  console.log("📡 Data from ESP:", req.body);

  lastData = {
    temperature: Number(req.body.temperature) || 0,
    pressure: Number(req.body.pressure) || 0,
    humidity: Number(req.body.humidity) || 0,
    light: Number(req.body.light) || 0,
    isRaining: Boolean(req.body.isRaining),
  };

  res.json({ status: "ok" });
});

// 📤 браузер забирает данные отсюда
app.get("/data", (req, res) => {
  res.json(lastData);
});

// 🌐 главная страница
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

// 🚀 запуск сервера (ВАЖНО для Railway)
const PORT = process.env.PORT || 8080;
app.listen(PORT, () => {
  console.log(`🚀 Server running on port ${PORT}`);
});
