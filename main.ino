#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

const char* ssid = "LapakAyam";
const char* password = "reifalkontol";
const char* serverUrl = "https://webiot-ardian56s-projects.vercel.app/latest-timer";

Servo servoMotor;
const int servoPin = 2;

void feed() {
    Serial.println("Feeding...");
    servoMotor.write(90);
    delay(5000);
    servoMotor.write(0);
}

bool parseAndCheckTime(const String& response) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.println("JSON parse error");
        return false;
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get current time");
        return false;
    }

    char currentTime[6];
    strftime(currentTime, sizeof(currentTime), "%H:%M", &timeinfo);

    String scheduleTime = doc["jam"].as<String>().substring(0, 5);
    Serial.println("Current time: " + String(currentTime) + ", Schedule: " + scheduleTime);

    return String(currentTime) == scheduleTime;
}

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");

    
    ESP32PWM::allocateTimer(0);
    servoMotor.setPeriodHertz(50);
    servoMotor.attach(servoPin, 500, 2400);
    servoMotor.write(0);


    configTime(25200, 0, "pool.ntp.org", "time.nist.gov");
    delay(2000);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Reconnecting...");
        WiFi.reconnect();
        delay(5000);
        return;
    }

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        Serial.println("Response: " + payload);

        if (parseAndCheckTime(payload)) {
            feed();
            delay(60000);
        }
    } else {
        Serial.println("HTTP Error: " + String(httpCode));
    }

    http.end();
    delay(1000);
}