#include <BME280_LITE.h>
#include <BH1750.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define BME_ADDR    0x76 
#define SEA_LEVEL_PRES  1017.8 

const char* ssid = "ser2.4";  
const char* password = "1807270100";

BH1750 lightMeter;
BME280_LITE bme;
ESP8266WebServer server(80);


struct SensorData {
  float temperature;
  float pressure;
  float humidity;
  float altitude;
  float light;
  bool tempValid;
  bool pressValid;
  bool humValid;
  bool altValid;
};

SensorData currentData;

void setup() {
  Serial.begin(115200);
  
  bool bmeInit = bme.begin(BME_ADDR, 
                          BME_H_X1,
                          BME_T_X1, 
                          BME_P_X16, 
                          BME_NORMAL, 
                          BME_TSB_0_5MS, 
                          BME_FILTER_2);
  
  if (bmeInit) {
    bme.calibrate(BME_ADDR);
    Serial.println("BME280 initialized successfully");
  } else {
    Serial.println("BME280 initialization failed!");
  }
  
  lightMeter.begin();
  if (lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE_2)) {
    Serial.println("BH1750 initialized successfully");
  } else {
    Serial.println("BH1750 initialization failed!");
  }

  // Подключение к WiFi
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // Ожидание подключения
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/update", handleUpdate);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  updateSensorData();
  delay(2000); // Обновление данных каждые 2 секунды
}

void updateSensorData() {
  // Температура
  BME_SensorData temperature = bme.readTemperature(BME_ADDR); 
  if (temperature.isValid) {
    currentData.temperature = temperature.data;
    currentData.tempValid = true;
  } else {
    currentData.tempValid = false;
    Serial.println("TEMP READ FAIL");
  }
  
  // Давление
  BME_SensorData pressure = bme.readPressure(BME_ADDR);
  if (pressure.isValid) {
    currentData.pressure = pressure.data;
    currentData.pressValid = true;
  } else {
    currentData.pressValid = false;
    Serial.println("PRESSURE READ FAIL");
  }

  // Влажность
  BME_SensorData humidity = bme.readHumidity(BME_ADDR);
  if (humidity.isValid) {
    currentData.humidity = humidity.data;
    currentData.humValid = true;
  } else {
    currentData.humValid = false;
    Serial.println("HUMIDITY READ FAIL");
  }
  
  // Высота
  BME_SensorData altitude = bme.readAltitude(BME_ADDR, SEA_LEVEL_PRES);
  if (altitude.isValid) {
    currentData.altitude = altitude.data;
    currentData.altValid = true;
  } else {
    currentData.altValid = false;
    Serial.println("ALTITUDE READ FAIL");
  }
  
  // Уровень освещенности
  currentData.light = lightMeter.readLightLevel();
}


// Сайт
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html lang='ru'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Датчики ESP12E</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background-color: #f0f0f0;
        }
        .container { 
            max-width: 800px; 
            margin: 0 auto; 
            background: white; 
            padding: 20px; 
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 { 
            text-align: center; 
            color: #333;
        }
        .sensor-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .sensor-card {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 10px;
            text-align: center;
        }
        .sensor-value {
            font-size: 24px;
            font-weight: bold;
            margin: 10px 0;
        }
        .sensor-unit {
            font-size: 14px;
            opacity: 0.8;
        }
        .error {
            color: #ff4444;
            background: #ffeaea;
            padding: 10px;
            border-radius: 5px;
            text-align: center;
        }
        .last-update {
            text-align: center;
            margin-top: 20px;
            color: #666;
            font-size: 12px;
        }
        .sensor-icon {
            font-size: 20px;
            margin-bottom: 10px;
        }
    </style>
</head>
<body>
    <div class='container'>
        <h1>&#128202; Показания датчиков</h1>
        <div id='sensorData' class='sensor-grid'>
            <!-- Данные будут загружены здесь -->
        </div>
        <div class='last-update'>
            Последнее обновление: <span id='lastUpdate'>--:--:--</span>
        </div>
    </div>

    <script>
        function loadData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    updateDisplay(data);
                    document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
                })
                .catch(error => {
                    console.error('Error:', error);
                    document.getElementById('sensorData').innerHTML = 
                        '<div class="error">Ошибка загрузки данных</div>';
                });
        }

        function updateDisplay(data) {
            const sensorData = document.getElementById('sensorData');
            let html = '';

            if (data.tempValid) {
                html += `
                    <div class="sensor-card">
                        <div class="sensor-icon">&#127777;</div>
                        <div>Температура</div>
                        <div class="sensor-value">${data.temperature.toFixed(1)}</div>
                        <div class="sensor-unit">°C</div>
                    </div>
                `;
            }

            if (data.pressValid) {
                html += `
                    <div class="sensor-card">
                        <div class="sensor-icon">&#128202;</div>
                        <div>Давление</div>
                        <div class="sensor-value">${data.pressure.toFixed(1)}</div>
                        <div class="sensor-unit">гПа</div>
                    </div>
                `;
            }

            if (data.humValid) {
                html += `
                    <div class="sensor-card">
                        <div class="sensor-icon">&#128167;</div>
                        <div>Влажность</div>
                        <div class="sensor-value">${data.humidity.toFixed(1)}</div>
                        <div class="sensor-unit">%</div>
                    </div>
                `;
            }

            if (data.altValid) {
                html += `
                    <div class="sensor-card">
                        <div class="sensor-icon">&#9968;</div>
                        <div>Высота</div>
                        <div class="sensor-value">${data.altitude.toFixed(1)}</div>
                        <div class="sensor-unit">метров</div>
                    </div>
                `;
            }

            html += `
                <div class="sensor-card">
                    <div class="sensor-icon">&#128161;</div>
                    <div>Освещенность</div>
                    <div class="sensor-value">${data.light.toFixed(0)}</div>
                    <div class="sensor-unit">люкс</div>
                </div>
            `;

            sensorData.innerHTML = html;
        }

        // Загружаем данные сразу при загрузке страницы
        loadData();
        
        // Обновляем данные каждые 3 секунды
        setInterval(loadData, 3000);
    </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(currentData.temperature) + ",";
  json += "\"pressure\":" + String(currentData.pressure) + ",";
  json += "\"humidity\":" + String(currentData.humidity) + ",";
  json += "\"altitude\":" + String(currentData.altitude) + ",";
  json += "\"light\":" + String(currentData.light) + ",";
  json += "\"tempValid\":" + String(currentData.tempValid ? "true" : "false") + ",";
  json += "\"pressValid\":" + String(currentData.pressValid ? "true" : "false") + ",";
  json += "\"humValid\":" + String(currentData.humValid ? "true" : "false") + ",";
  json += "\"altValid\":" + String(currentData.altValid ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleUpdate() {
  updateSensorData();
  server.send(200, "text/plain", "Data updated");
}
