const express = require("express");
const path = require("path");

const app = express();
app.use(express.json());

/* ================== ДАННЫЕ ОТ ESP ================== */
let sensorData = {
  temperature: 0,
  pressure: 0,
  humidity: 0,
  light: 0,
  isRaining: false,
  updatedAt: null
};

/* ================== HEALTH CHECK ================== */
app.get("/health", (req, res) => {
  res.status(200).send("OK");
});

/* ================== ESP -> POST /data ================== */
app.post("/data", (req, res) => {
  console.log("📡 Data from ESP:", req.body);

  sensorData = {
    ...req.body,
    updatedAt: new Date().toISOString()
  };

  res.status(200).json({ status: "ok" });
});

/* ================== SITE -> GET /data ================== */
app.get("/data", (req, res) => {
  // 🔥 отключаем кеш полностью
  res.setHeader("Cache-Control", "no-store, no-cache, must-revalidate, proxy-revalidate");
  res.setHeader("Pragma", "no-cache");
  res.setHeader("Expires", "0");

  res.json(sensorData);
});

/* ================== MAIN PAGE ================== */
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

/* ================== START SERVER ================== */
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log("🚀 Server running on port", PORT);
});
