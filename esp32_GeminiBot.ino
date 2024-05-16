#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* apiKey = "";

String endpoint = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + String(apiKey);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.println("Available WiFi networks:");
  for (int i = 0; i < n; ++i) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(")");
  }

  Serial.println("Select a WiFi network (Enter number): ");
  while (!Serial.available());
  int networkIndex = Serial.parseInt() - 1;
  while (networkIndex < 0 || networkIndex >= n) {
    Serial.println("Invalid selection. Try again: ");
    while (!Serial.available());
    networkIndex = Serial.parseInt() - 1;
  }

  String ssid = WiFi.SSID(networkIndex);
  Serial.print("Selected SSID: ");
  Serial.println(ssid);
  
  Serial.println("Enter password: ");
  while (!Serial.available());
  String password = Serial.readStringUntil('\n');
  password.trim();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  int retries = 0;
  const int maxRetries = 20; 
  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Error: Could not connect to WiFi network.");
    ESP.restart();
  }
}

void loop() {
  if (Serial.available() > 0) {
    String userQuery = Serial.readStringUntil('\n');
    userQuery.trim();

    if (userQuery.length() > 0) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        http.setTimeout(15000);
        http.begin(endpoint);
        http.addHeader("Content-Type", "application/json");
        
        String payload = "{\"contents\":[{\"parts\":[{\"text\":\"" + userQuery + "\"}]}]}";
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
          String response = http.getString();

          DynamicJsonDocument doc(4096);
          DeserializationError error = deserializeJson(doc, response);

          if (!error) {
            const char* text = doc["candidates"][0]["content"]["parts"][0]["text"];
            Serial.print("You: \"");
            Serial.print(userQuery);
            Serial.println("\"");
            Serial.print("Gemini: \"");
            Serial.print(text);
            Serial.println("\"");
          } else {
            Serial.print("Error parsing JSON: ");
            Serial.println(error.c_str());
          }
        } else {
          Serial.print("Request error: ");
          Serial.println(httpResponseCode);
          Serial.print("HTTP error: ");
          Serial.println(http.errorToString(httpResponseCode).c_str());

          if (httpResponseCode == HTTP_CODE_UNAUTHORIZED) {
            Serial.println("Error: Unauthorized. Check your API key.");
          } else if (httpResponseCode == HTTP_CODE_FORBIDDEN) {
            Serial.println("Error: Forbidden. You do not have permission to access this resource.");
          } else if (httpResponseCode == HTTP_CODE_NOT_FOUND) {
            Serial.println("Error: Resource not found.");
          } else if (httpResponseCode == HTTP_CODE_REQUEST_TIMEOUT) {
            Serial.println("Error: Request timed out.");
          } else if (httpResponseCode == HTTP_CODE_INTERNAL_SERVER_ERROR) {
            Serial.println("Error: Internal server error.");
          } else {
            Serial.println("Unknown error.");
          }

          delay(2000);
          Serial.println("Retrying...");
        }

        http.end();
      } else {
        Serial.println("WiFi connection error");
      }
    }
  }
}
