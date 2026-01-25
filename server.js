const express = require("express");
const fs = require("fs");
const path = require("path");

const app = express();
app.use(express.json());

const DATA_FILE = path.join(__dirname, "data.json");

// если файла нет — создаём
if (!fs.existsSync(DATA_FILE)) {
  fs.writeFileSync(
    DATA_FILE,
    JSON.stringify({
      temperature: 0,
      pressure: 0,
      humidity: 0,
      light: 0,
      isRaining: false
    }, null, 2)
  );
}

// 📥 ESP → POST /data
app.post("/data", (req, res) => {
  console.log("📡 Data from ESP:", req.body);

  const data = {
    temperature: Number(req.body.temperature) || 0,
    pressure: Number(req.body.pressure) || 0,
    humidity: Number(req.body.humidity) || 0,
    light: Number(req.body.light) || 0,
    isRaining: Boolean(req.body.isRaining)
  };

  fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2));
  res.json({ status: "ok" });
});

// 📤 браузер → GET /data
app.get("/data", (req, res) => {
  const data = JSON.parse(fs.readFileSync(DATA_FILE));
  res.json(data);
});

// 🌍 главная страница
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

// 🚀 запуск (ВАЖНО для Railway)
const PORT = process.env.PORT || 8080;
app.listen(PORT, () => {
  console.log(`🚀 Server running on port ${PORT}`);
});
