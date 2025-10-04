/******************************************************
 * Project: EcoSphere Monitor
 * Description: IoT-Based Air Quality and Gas Monitoring System
 * Board: ESP32
 * Author: Joshua Muthenya Wambua
 ******************************************************/

// ---------- Include Required Libraries ----------
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <SoftwareSerial.h>  // For PM sensor if using UART communication

// ---------- Define Firebase & Wi-Fi Credentials ----------
#define WIFI_SSID       "your_wifi_name"
#define WIFI_PASSWORD   "your_wifi_password"

#define FIREBASE_HOST   "https://your-project.firebaseio.com/"
#define FIREBASE_AUTH   "your_firebase_database_secret"

// ---------- Define Sensor Pins ----------
#define DHTPIN  4
#define DHTTYPE DHT11

#define MQ2_PIN   34
#define MQ7_PIN   35
#define MQ9_PIN   32
#define MQ135_PIN 33
#define BUZZER_PIN 15
#define LED_PIN 2

// ---------- Initialize Objects ----------
DHT dht(DHTPIN, DHTTYPE);
FirebaseData fbData;

// ---------- PM Sensor Setup (Nova PM) ----------
SoftwareSerial pmsSerial(16, 17); // RX, TX pins for PM sensor
int pm25, pm10;

// ---------- Setup Function ----------
void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600);
  dht.begin();
  
  pinMode(MQ2_PIN, INPUT);
  pinMode(MQ7_PIN, INPUT);
  pinMode(MQ9_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Connected!");
}

// ---------- Function to Read PM Sensor ----------
void readPMSensor() {
  if (pmsSerial.available() > 0) {
    // Example placeholder for reading PM data from Nova PM sensor
    pm25 = analogRead(36); // Replace with actual serial frame parsing
    pm10 = analogRead(39);
  }
}

// ---------- Function to Send Data to Firebase ----------
void sendToFirebase(float temp, float hum, int mq2, int mq7, int mq9, int mq135, int pm25, int pm10) {
  FirebaseJson json;
  json.add("temperature", temp);
  json.add("humidity", hum);
  json.add("mq2", mq2);
  json.add("mq7", mq7);
  json.add("mq9", mq9);
  json.add("mq135", mq135);
  json.add("pm25", pm25);
  json.add("pm10", pm10);
  json.add("timestamp", millis());

  if (Firebase.pushJSON(fbData, "/EcoSphere/data", json)) {
    Serial.println("Data uploaded successfully.");
  } else {
    Serial.println("Firebase upload failed: " + fbData.errorReason());
  }
}

// ---------- Main Loop ----------
void loop() {
  // Read sensors
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  int mq2 = analogRead(MQ2_PIN);
  int mq7 = analogRead(MQ7_PIN);
  int mq9 = analogRead(MQ9_PIN);
  int mq135 = analogRead(MQ135_PIN);

  readPMSensor();

  // Display readings
  Serial.printf("Temp: %.2f°C | Hum: %.2f%% | MQ2: %d | MQ7: %d | MQ9: %d | MQ135: %d | PM2.5: %d | PM10: %d\n",
                temp, hum, mq2, mq7, mq9, mq135, pm25, pm10);

  // Threshold-based alert example
  if (mq2 > 600 || mq135 > 700 || pm25 > 80) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("⚠️ ALERT: Poor air quality detected!");
  }

  // Upload to Firebase
  sendToFirebase(temp, hum, mq2, mq7, mq9, mq135, pm25, pm10);

  delay(5000); // Wait before next reading
}
