const express = require("express");
const app = express();

app.use(express.json());

// Хранилище последних данных
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

/* ===== ESP отправляет данные ===== */
app.post("/data", (req, res) => {
  sensorData = req.body;
  console.log("📡 Data from ESP:", sensorData);
  res.send({ status: "ok" });
});

/* ===== Браузер читает данные ===== */
app.get("/data", (req, res) => {
  res.json(sensorData);
});

/* ===== САЙТ ===== */
app.get("/", (req, res) => {
  res.sendFile(__dirname + "/index.html");
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log("🚀 Server running on port", PORT);
});
