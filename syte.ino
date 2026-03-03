/*
ESP8266 + BME280 + BH1750
Отправка данных на Railway (HTTPS, POST /data)
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BME280_LITE.h>
#include <BH1750.h>

/* ========== WIFI ========== */
const char* ssid = "ser2.4";
const char* password = "1807270100";

/* ========== RAILWAY ========== */
const char* serverHost = "weatherstation-production-852e.up.railway.app";

/* ========== OPENWEATHER (дождь) ========== */
const char* weatherHost = "api.openweathermap.org";
const char* weatherApiKey = "29bba9ae078372bd5ed627dac010eadc";  // get на openweathermap.org
const char* city = "Moscow";

/* ========== SENSORS ========== */
#define BME_ADDR 0x76
BME280_LITE bme;
BH1750 lightMeter;

/* ========== TIMING ========== */
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 10000; // 10 seconds

/* ========== DATA ========== */
float temperature = 0;
float pressure = 0;
float humidity = 0;
float light = 0;
bool isRaining = false;

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(100);

  /* --- WiFi --- */
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  /* --- BME280 --- */
  bme.begin(BME_ADDR, BME_H_X1, BME_T_X1, BME_P_X16,
            BME_NORMAL, BME_TSB_0_5MS, BME_FILTER_2);
  bme.calibrate(BME_ADDR);

  /* --- BH1750 --- */
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2);

  updateSensors();
  checkRain();
  sendToRailway();  // Тестовая отправка при старте
}

/* ================= LOOP ================= */
void loop() {
  if (millis() - lastUpdate > UPDATE_INTERVAL) {
    lastUpdate = millis();
    updateSensors();
    checkRain();
    sendToRailway();
    Serial.println("--- Цикл обновления завершен ---");
  }
  delay(1000);
}

/* ================= SENSORS ================= */
void updateSensors() {
  auto t = bme.readTemperature(BME_ADDR);
  if (t.isValid) temperature = t.data;
  auto p = bme.readPressure(BME_ADDR);
  if (p.isValid) pressure = p.data * 0.750061683;
  auto h = bme.readHumidity(BME_ADDR);
  if (h.isValid) humidity = h.data;
  light = lightMeter.readLightLevel();

  Serial.print("📊 Данные: T="); Serial.print(temperature);
  Serial.print(" P="); Serial.print(pressure);
  Serial.print(" H="); Serial.print(humidity);
  Serial.print(" L="); Serial.println(light);
}

/* ================= RAIN ================= */
void checkRain() {
  WiFiClient client;
  isRaining = false;
  if (!client.connect(weatherHost, 80)) {
    Serial.println("❌ OpenWeather connect failed");
    return;
  }

  String url = "/data/2.5/weather?q=" + String(city) +
               "&appid=" + weatherApiKey;
  client.print(
    "GET " + url + " HTTP/1.1\r\n"
    "Host: " + String(weatherHost) + "\r\n"
    "Connection: close\r\n\r\n"
  );

  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf("\"rain\"") >= 0 ||
          line.indexOf("\"snow\"") >= 0) {
        isRaining = true;
      }
    }
  }
  client.stop();
  Serial.print("🌧️ Дождь: "); Serial.println(isRaining ? "да" : "нет");
}

/* ================= SEND TO RAILWAY ================= */
void sendToRailway() {
  WiFiClientSecure client;
  client.setInsecure();
  Serial.println("Connecting to Railway (HTTPS)...");
  if (!client.connect(serverHost, 443)) {
    Serial.println("❌ HTTPS connection FAILED");
    return;
  }
  Serial.println("✅ Connected");

  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"pressure\":" + String(pressure) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"light\":" + String(light) + ",";
  json += "\"isRaining\":" + String(isRaining ? "true" : "false");
  json += "}";

  Serial.print("📤 Отправка JSON: "); Serial.println(json);

  client.print(
    "POST /data HTTP/1.1\r\n"
    "Host: " + String(serverHost) + "\r\n"
    "User-Agent: ESP8266\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: " + String(json.length()) + "\r\n"
    "Connection: close\r\n\r\n" +
    json
  );

  // Читаем ответ
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 5000) {
    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println("← Сервер: " + line);
    }
  }

  client.stop();
  Serial.println("✅ POST completed");
}
