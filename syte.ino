/*
 ESP8266 + BME280 + BH1750
 Pressure in mmHg
 Rain from OpenWeatherMap
 Clouds NEVER overlap sensor data
*/

#include <BME280_LITE.h>
#include <BH1750.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define BME_ADDR 0x76

const char* ssid = "Имя сети";
const char* password = "Пароль";
//  Нужна сеть с 2,4 ГГЦ

/* OpenWeatherMap */
const char* weatherHost = "api.openweathermap.org";
const char* weatherApiKey = "ТВОЙ_API_KEY";
const char* city = "Moscow";

BH1750 lightMeter;
BME280_LITE bme;
ESP8266WebServer server(80);

/* ===== обновление ===== */
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 180000;

/* ===== данные ===== */
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

  bme.begin(BME_ADDR, BME_H_X1, BME_T_X1, BME_P_X16,
            BME_NORMAL, BME_TSB_0_5MS, BME_FILTER_2);
  bme.calibrate(BME_ADDR);

  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  updateSensorData();
  checkRain();
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();

  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    updateSensorData();
    checkRain();
  }
}

/* ================= SENSORS ================= */
void updateSensorData() {
  auto t = bme.readTemperature(BME_ADDR);
  if ((currentData.tempValid = t.isValid))
    currentData.temperature = t.data;

  auto p = bme.readPressure(BME_ADDR);
  if ((currentData.pressValid = p.isValid))
    currentData.pressure = p.data * 0.750061683;

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

/* ================= WEB ================= */
void handleRoot() {
String html = R"=====(<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP8266 Метеостанция</title>

<style>
*{box-sizing:border-box}
body{
 margin:0;
 background:linear-gradient(180deg,#0b0b14,#151528);
 color:#fff;
 font-family:-apple-system,BlinkMacSystemFont,Segoe UI,Roboto;
 overflow-x:hidden;
}

/* ===== ЗОНА ТУЧ (НИЖЕ ДАННЫХ) ===== */
.sky{
 position:absolute;
 top:450px;          /* 🔽 ниже данных */
 left:0;
 width:100%;
 height:200px;
 pointer-events:none;
}

/* ===== CLOUD ===== */
.cloud{
 position:absolute;
 width:160px;
 height:60px;
 background:#fff;
 border-radius:50px;
 opacity:.95;
}

.cloud:before,.cloud:after{
 content:'';
 position:absolute;
 background:#fff;
 width:80px;
 height:80px;
 border-radius:50%;
 top:-35px;
}
.cloud:before{left:20px}
.cloud:after{right:20px}

.rainy .cloud,
.rainy .cloud:before,
.rainy .cloud:after{
 background:#9aa0a6;
}

/* ===== CLOUD MOTION ===== */
@media (min-width:768px){
 .cloud{animation:floatCloud linear infinite alternate}
 .c1{top:10px;left:5%;animation-duration:22s}
 .c2{top:50px;left:30%;animation-duration:26s}
 .c3{top:90px;left:55%;animation-duration:24s}
 .c4{top:130px;left:15%;animation-duration:30s}
 .c5{top:40px;left:70%;animation-duration:28s}
 .c6{top:110px;left:85%;animation-duration:32s}
}

@keyframes floatCloud{
 from{transform:translateX(-40px)}
 to{transform:translateX(40px)}
}

/* ===== RAIN ===== */
.rain{
 position:absolute;
 top:60px;
 left:0;
 width:100%;
 height:140px;
 display:none;
}
.rainy .rain{display:block}

.drop{
 position:absolute;
 width:2px;
 height:14px;
 background:rgba(180,200,255,.85);
 animation:fall linear infinite;
}

@keyframes fall{
 to{transform:translateY(140px);opacity:0}
}

/* ===== CONTENT ===== */
.content{
 margin-top:40px;    /* данные всегда выше */
}

h1{text-align:center;font-size:22px;margin:16px 0}

.grid{
 display:grid;
 grid-template-columns:repeat(auto-fit,minmax(150px,1fr));
 gap:12px;
 padding:12px
}

.card{
 background:#1e1e2f;
 border-radius:18px;
 padding:14px;
 text-align:center;
 box-shadow:0 6px 16px rgba(0,0,0,.45)
}

.icon{font-size:32px}
.title{font-size:14px;opacity:.7}
.value{font-size:26px;font-weight:700;margin:6px 0}
.unit{font-size:13px;opacity:.6}
</style>
</head>

<body>

<div class="content">
<h1>📡 Метеостанция</h1>
<div id="grid" class="grid"></div>
</div>

<div class="sky" id="sky">
 <div class="cloud c1"></div>
 <div class="cloud c2"></div>
 <div class="cloud c3"></div>
 <div class="cloud c4"></div>
 <div class="cloud c5"></div>
 <div class="cloud c6"></div>
 <div class="rain" id="rain"></div>
</div>

<script>
const grid=document.getElementById('grid');
const sky=document.getElementById('sky');
const rain=document.getElementById('rain');

function card(i,t,v,u){
 return `<div class="card">
 <div class="icon">${i}</div>
 <div class="title">${t}</div>
 <div class="value">${v}</div>
 <div class="unit">${u}</div>
 </div>`;
}

function makeRain(){
 rain.innerHTML='';
 for(let i=0;i<60;i++){
  let d=document.createElement('div');
  d.className='drop';
  d.style.left=Math.random()*100+'%';
  d.style.animationDuration=(0.5+Math.random())+'s';
  d.style.animationDelay=Math.random()+'s';
  rain.appendChild(d);
 }
}

function load(){
 fetch('/data').then(r=>r.json()).then(d=>{
  let h='';
  if(d.tempValid) h+=card("🌡️","Температура",d.temperature.toFixed(1),"°C");
  if(d.pressValid) h+=card("🧭","Давление",d.pressure.toFixed(1),"мм рт. ст.");
  if(d.humValid) h+=card("💧","Влажность",d.humidity.toFixed(1),"%");
  h+=card(d.isRaining?"🌧️":"☀️","Осадки",d.isRaining?"Да":"Нет","");
  h+=card("💡","Освещённость",d.light.toFixed(0),"лк");
  grid.innerHTML=h;

  if(d.isRaining){
    sky.classList.add('rainy');
    makeRain();
  } else {
    sky.classList.remove('rainy');
  }
 });
}

load();
setInterval(load,4000);
</script>

</body>
</html>)=====";

server.send(200,"text/html",html);
}

/* ================= JSON ================= */
void handleData() {
 String j="{";
 j+="\"temperature\":"+String(currentData.temperature)+",";
 j+="\"pressure\":"+String(currentData.pressure)+",";
 j+="\"humidity\":"+String(currentData.humidity)+",";
 j+="\"light\":"+String(currentData.light)+",";
 j+="\"isRaining\":"+String(currentData.isRaining?"true":"false")+",";
 j+="\"tempValid\":"+String(currentData.tempValid?"true":"false")+",";
 j+="\"pressValid\":"+String(currentData.pressValid?"true":"false")+",";
 j+="\"humValid\":"+String(currentData.humValid?"true":"false");
 j+="}";
 server.send(200,"application/json",j);
}
