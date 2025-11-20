
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// --- Your Wi-Fi Credentials ---
const char* ssid = "oneplus";
const char* password = "harsh12345";

// --- Google Script URL ---
String scriptURL = "https://script.google.com/macros/s/AKfycbxYME82xSdgBMYv6-aWCO3GU3MN29SZAj3h8GTcOqbIIBEyUARN8ODSwQ1nyQh2IRMpyw/exec"; 

// --- Pin Definitions ---
// L298N Motor Driver
#define ENA D2
#define IN1 D3
#define IN2 D4
#define ENB D8
#define IN3 D6
#define IN4 D7
// Sensors
#define DHTPIN D5
#define DHTTYPE DHT22
#define SOIL_PIN A0

// --- Global Objects ---
ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

// --- Variables for Database Timer ---
unsigned long previousMillis = 0;
const long interval = 60000; // Log to spreadsheet every 1 minute

// --- Combined HTML, CSS, & JavaScript for the Web Page ---
String getCombinedPage() {
 String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>IoT Robot & Sensor Hub</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta charset="UTF-8">
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;700&display=swap" rel="stylesheet">
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>
    :root {
        --primary-color: #007bff;
        --secondary-color: #343a40; 
        --card-background: rgba(255, 255, 255, 0.9);
        --text-color: #333;
        --shadow-color: rgba(0,0,0,0.15);
    }
    body { 
        font-family: 'Roboto', sans-serif; 
        background-image: linear-gradient(rgba(255, 255, 255, 0.8), rgba(255, 255, 255, 0.8)), url(https://i.ibb.co/Jq03VwJ/7a0b5107e584.jpg);
        background-size: cover; background-position: center; background-attachment: fixed;
        margin: 0; padding: 20px; color: var(--text-color); 
    }
    h1, h2 { text-align: center; color: var(--primary-color); font-weight: 700; }
    .section-title { border-bottom: 2px solid var(--primary-color); padding-bottom: 10px; margin-bottom: 20px; }
    .dashboard, .robot-controls { margin-bottom: 30px; }
    .card-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; }
    .card { background-color: var(--card-background); border-radius: 15px; box-shadow: 0 8px 25px var(--shadow-color); padding: 25px; text-align: center; transition: transform 0.2s ease-in-out; border-top: 4px solid var(--primary-color); }
    .card:hover { transform: translateY(-5px); }
    .card h3 { margin-top: 0; margin-bottom: 15px; color: var(--primary-color); font-size: 1.2em; font-weight: 400; display: flex; align-items: center; justify-content: center; }
    .card h3 span { margin-right: 10px; }
    .card .value { font-size: 2.8em; font-weight: 700; margin: 10px 0; line-height: 1; }
    .card .unit { font-size: 1.2em; color: var(--secondary-color); font-weight: 300; }
    .charts-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
    .chart-container { background-color: var(--card-background); padding: 20px; border-radius: 15px; box-shadow: 0 8px 25px var(--shadow-color); }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr 1fr; max-width: 300px; margin: 20px auto; gap: 15px; }
    .btn { border: none; padding: 20px; font-size: 28px; border-radius: 12px; color: #2c3e50; background-color: #ecf0f1; cursor: pointer; touch-action: manipulation; }
    .btn:active { background-color: #bdc3c7; }
    .forward { grid-column: 2; }
    .left { grid-column: 1; }
    .right { grid-column: 3; }
    .reverse { grid-column: 2; }
    .stop { grid-column: 1 / 4; background-color: #e74c3c; color: white; }
</style>
</head>
<body>
    <h1>🤖 IoT Robot & Sensor Hub</h1>
    
    <div class="dashboard">
        <h2 class="section-title">Sensor Dashboard</h2>
        <div class="card-grid">
            <div class="card"><h3><span>🌡️</span>Temperature</h3><p class="value" id="temp">--</p><p class="unit">&deg;C</p></div>
            <div class="card"><h3><span>💧</span>Humidity</h3><p class="value" id="hum">--</p><p class="unit">%</p></div>
            <div class="card"><h3><span>🌱</span>Soil Moisture</h3><p class="value" id="soil">--</p><p class="unit">%</p></div>
            <div class="card"><h3><span>📝</span>Soil Status</h3><p class="value" id="soil_status" style="font-size: 1.5em;">--</p><p class="unit">🌵 / 💧</p></div>
        </div>
    </div>

    <div class="robot-controls">
        <h2 class="section-title">Robot Control</h2>
        <div class="btn-grid">
            <button class="btn forward" onmousedown="location.href='/forward'" onmouseup="location.href='/stop'" ontouchstart="location.href='/forward'" ontouchend="location.href='/stop'">&#8593;</button>
            <button class="btn left"    onmousedown="location.href='/left'"    onmouseup="location.href='/stop'" ontouchstart="location.href='/left'"    ontouchend="location.href='/stop'">&#8592;</button>
            <div></div> 
            <button class="btn right"   onmousedown="location.href='/right'"   onmouseup="location.href='/stop'" ontouchstart="location.href='/right'"   ontouchend="location.href='/stop'">&#8594;</button>
            <button class="btn reverse" onmousedown="location.href='/reverse'" onmouseup="location.href='/stop'" ontouchstart="location.href='/reverse'" ontouchend="location.href='/stop'">&#8595;</button>
            <button class="btn stop"    onclick="location.href='/stop'">STOP</button>
        </div>
    </div>

    <div class="charts-grid">
        <div class="chart-container"><canvas id="tempChart"></canvas></div>
        <div class="chart-container"><canvas id="humidityChart"></canvas></div>
        <div class="chart-container"><canvas id="soilChart"></canvas></div>
    </div>

    <script>
        const MAX_DATA_POINTS = 20;
        const labels = [];
        const tempData = [], humidityData = [], soilData = [];
        const tempChartCtx = document.getElementById('tempChart').getContext('2d');
        const tempChart = new Chart(tempChartCtx, { type: 'line', data: { labels: labels, datasets: [{ label: 'Temperature (°C)', data: tempData, borderColor: 'rgba(255, 99, 132, 1)', backgroundColor: 'rgba(255, 99, 132, 0.2)', borderWidth: 2, fill: true }] }, options: { responsive: true, scales: { y: { title: { display: true, text: '°C' } } } } });
        const humidityChartCtx = document.getElementById('humidityChart').getContext('2d');
        const humidityChart = new Chart(humidityChartCtx, { type: 'line', data: { labels: labels, datasets: [{ label: 'Humidity (%)', data: humidityData, borderColor: 'rgba(54, 162, 235, 1)', backgroundColor: 'rgba(54, 162, 235, 0.2)', borderWidth: 2, fill: true }] }, options: { responsive: true, scales: { y: { min: 0, max: 100, title: { display: true, text: '%' } } } } });
        const soilChartCtx = document.getElementById('soilChart').getContext('2d');
        const soilChart = new Chart(soilChartCtx, { type: 'line', data: { labels: labels, datasets: [{ label: 'Soil Moisture (%)', data: soilData, borderColor: 'rgba(75, 192, 192, 1)', backgroundColor: 'rgba(75, 192, 192, 0.2)', borderWidth: 2, fill: true }] }, options: { responsive: true, scales: { y: { min: 0, max: 100, title: { display: true, text: '%' } } } } });
        
        function addDataToCharts(data) {
            const now = new Date();
            const timeString = now.getHours().toString().padStart(2, '0') + ':' + now.getMinutes().toString().padStart(2, '0') + ':' + now.getSeconds().toString().padStart(2, '0');
            labels.push(timeString);
            tempData.push(data.temperature);
            humidityData.push(data.humidity);
            soilData.push(data.soil_moisture);
            if (labels.length > MAX_DATA_POINTS) {
                labels.shift(); tempData.shift(); humidityData.shift(); soilData.shift();
            }
            tempChart.update(); humidityChart.update(); soilChart.update();
        }
        function fetchData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temp').innerText = data.temperature.toFixed(1);
                    document.getElementById('hum').innerText = data.humidity.toFixed(1);
                    document.getElementById('soil').innerText = data.soil_moisture;
                    document.getElementById('soil_status').innerText = data.soil_status;
                    addDataToCharts(data);
                })
                .catch(error => console.error('Error fetching data:', error));
        }
        setInterval(fetchData, 5000);
        window.onload = fetchData;
    </script>
</body>
</html>
)rawliteral";
 return page;
}

// --- Motor Control Functions ---
void moveForward()  { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void moveBackward() { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);}
void turnLeft()     { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void turnRight()    { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);}
void stopMotors()   { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW); digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW); }

// --- Function to send data to Google Sheets ---
void sendToDatabase() {
  Serial.println("\nReading sensor data for database update...");
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soilValue = analogRead(SOIL_PIN);
  int soilPercent = map(soilValue, 950, 300, 0, 100);
  if (soilPercent < 0) soilPercent = 0;
  if (soilPercent > 100) soilPercent = 100;

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor! Skipping database update.");
    return;
  }

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  String urlWithParams = scriptURL + "?temp=" + String(temperature) + "&hum=" + String(humidity) + "&soil=" + String(soilPercent);
  Serial.print("Sending data to Google Sheet...");
  if (http.begin(client, urlWithParams)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("... Done! Response code: %d\n", httpCode);
    } else {
      Serial.printf("... Failed! Error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("... Failed to connect to URL!");
  }
}

// --- Web Server Handler Functions ---
void handleRoot()     { server.send(200, "text/html", getCombinedPage()); }
void handleForward()  { moveForward();  server.send(200, "text/plain", "Forward"); }
void handleReverse()  { moveBackward(); server.send(200, "text/plain", "Backward");}
void handleLeft()     { turnLeft();     server.send(200, "text/plain", "Left");    }
void handleRight()    { turnRight();    server.send(200, "text/plain", "Right");   }
void handleStop()     { stopMotors();   server.send(200, "text/plain", "Stop");    }
void handleData() {
  int soilValue = analogRead(SOIL_PIN);
  int soilPercent = map(soilValue, 950, 300, 0, 100); 
  if (soilPercent < 0) soilPercent = 0;
  if (soilPercent > 100) soilPercent = 100;

  String soilStatus = (soilPercent < 40) ? "Soil is Dry 🌵" : "Soil is Wet 💧";
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    temperature = 0.0; humidity = 0.0;
  }
  
  String json = "{\"temperature\":" + String(temperature) + 
                ",\"humidity\":" + String(humidity) + 
                ",\"soil_moisture\":" + String(soilPercent) + 
                ",\"soil_status\":\"" + soilStatus + "\"}";
  server.send(200, "application/json", json);
}

// --- Main Setup Function ---
void setup() {
  Serial.begin(115200);
  
  // Initialize Sensors
  dht.begin();

  // Initialize Motor Pins
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // Set initial motor speed (0-1023)
  analogWrite(ENA, 800);
  analogWrite(ENB, 800);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  // Define Server Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/forward", HTTP_GET, handleForward);
  server.on("/reverse", HTTP_GET, handleReverse);
  server.on("/left", HTTP_GET, handleLeft);
  server.on("/right", HTTP_GET, handleRight);
  server.on("/stop", HTTP_GET, handleStop);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

// --- Main Loop Function ---
void loop() {
  server.handleClient(); // Listen for web requests

  // Timer for logging data to Google Sheets
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    sendToDatabase();
  }
}