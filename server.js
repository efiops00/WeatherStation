const express = require("express");
const path = require("path");

const app = express();
app.use(express.json());

// ⚠️ ГЛОБАЛЬНОЕ ХРАНЕНИЕ
global.weatherData = {
  temperature: 0,
  pressure: 0,
  humidity: 0,
  light: 0,
  isRaining: false
};

// ESP → POST
app.post("/data", (req, res) => {
  console.log("📡 ESP DATA:", req.body);

  global.weatherData = {
    temperature: Number(req.body.temperature),
    pressure: Number(req.body.pressure),
    humidity: Number(req.body.humidity),
    light: Number(req.body.light),
    isRaining: Boolean(req.body.isRaining)
  };

  res.sendStatus(200);
});

// браузер → GET
app.get("/data", (req, res) => {
  res.json(global.weatherData);
});

// HTML
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

const PORT = process.env.PORT || 8080;
app.listen(PORT, () => {
  console.log("🚀 Server running on", PORT);
});
