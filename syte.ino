/*
 ESP8266 + BME280 + BH1750
 Отправка данных на Railway
 Давление в мм рт. ст.
 Осадки из OpenWeatherMap
*/

#include <ESP8266WiFi.h>
#include <BME280_LITE.h>
#include <BH1750.h>

/* ===== WIFI ===== */
const char* ssid = "ser2.4";
const char* password = "1807270100";

/* ===== RAILWAY ===== */
const char* serverHost = "https://weatherstation-production-17f7.up.railway.app"; // ← ЗАМЕНИ
const int serverPort = 80;

/* ===== OPENWEATHER ===== */
const char* weatherHost = "api.openweathermap.org";
const char* weatherApiKey = "ТВОЙ_API_KEY";
const char* city = "Moscow";

/* ===== SENSORS ===== */
#define BME_ADDR 0x76
BME280_LITE bme;
BH1750 lightMeter;

/* ===== UPDATE ===== */
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 180000; // 3 минуты

/* ===== DATA ===== */
struct SensorData {
  float temperature = 0;
  float pressure = 0;
  float humidity = 0;
  float light = 0;
  bool tempValid = false;
  bool pressValid = false;
  bool humValid = false;
  bool isRaining = false;
};

SensorData currentData;

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(100);

  /* --- BME280 --- */
  bme.begin(BME_ADDR, BME_H_X1, BME_T_X1, BME_P_X16,
            BME_NORMAL, BME_TSB_0_5MS, BME_FILTER_2);
  bme.calibrate(BME_ADDR);

  /* --- BH1750 --- */
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2);

  /* --- WIFI --- */
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  updateSensorData();
  checkRain();
  sendToRailway();
}

/* ================= LOOP ================= */
void loop() {
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    updateSensorData();
    checkRain();
    sendToRailway();
  }
}

/* ================= SENSORS ================= */
void updateSensorData() {
  auto t = bme.readTemperature(BME_ADDR);
  if ((currentData.tempValid = t.isValid))
    currentData.temperature = t.data;

  auto p = bme.readPressure(BME_ADDR);
  if ((currentData.pressValid = p.isValid))
    currentData.pressure = p.data * 0.750061683; // Pa → mmHg

  auto h = bme.readHumidity(BME_ADDR);
  if ((currentData.humValid = h.isValid))
    currentData.humidity = h.data;

  currentData.light = lightMeter.readLightLevel();
}

/* ================= RAIN ================= */
void checkRain() {
  WiFiClient client;
  currentData.isRaining = false;

  if (!client.connect(weatherHost, 80)) return;

  String url = "/data/2.5/weather?q=" + String(city) +
               "&appid=" + weatherApiKey;

  client.print(
    "GET " + url + " HTTP/1.1\r\n"
    "Host: " + weatherHost + "\r\n"
    "Connection: close\r\n\r\n"
  );

  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf("\"rain\"") >= 0 ||
          line.indexOf("\"snow\"") >= 0) {
        currentData.isRaining = true;
      }
    }
  }
}

/* ================= SEND TO RAILWAY ================= */
void sendToRailway() {
  WiFiClient client;

  if (!client.connect(serverHost, serverPort)) {
    Serial.println("Railway connection failed");
    return;
  }

  String json = "{";
  json += "\"temperature\":" + String(currentData.temperature) + ",";
  json += "\"pressure\":" + String(currentData.pressure) + ",";
  json += "\"humidity\":" + String(currentData.humidity) + ",";
  json += "\"light\":" + String(currentData.light) + ",";
  json += "\"isRaining\":" + String(currentData.isRaining ? "true" : "false") + ",";
  json += "\"tempValid\":" + String(currentData.tempValid ? "true" : "false") + ",";
  json += "\"pressValid\":" + String(currentData.pressValid ? "true" : "false") + ",";
  json += "\"humValid\":" + String(currentData.humValid ? "true" : "false");
  json += "}";

  client.print(
    "POST /data HTTP/1.1\r\n"
    "Host: " + String(serverHost) + "\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: " + String(json.length()) + "\r\n"
    "Connection: close\r\n\r\n" +
    json
  );

  Serial.println("📡 Sent to Railway:");
  Serial.println(json);
}
