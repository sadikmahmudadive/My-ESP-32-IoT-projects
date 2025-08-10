#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Configuration ---
const char* ssid = "IoTbaseLow";
const char* password = "01835120307";
String geminiApiKey = "AIzaSyB8T-nIt06t-Y9UHxUcaknFY9c_m7F2hV8";

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Gemini API URL
String geminiApiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + geminiApiKey;

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Connecting to WiFi...");
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected!");
  display.println("Ready for prompt...");
  display.display();

  Serial.println("\nEnter your prompt in the Serial Monitor:");
}

void loop() {
  if (Serial.available() > 0) {
    String prompt = Serial.readStringUntil('\n');
    prompt.trim();
    if (prompt.length() > 0) {
      Serial.print("User prompt: ");
      Serial.println(prompt);

      displayMessage("Thinking...");

      String response = callGeminiAPI(prompt);

      displayMessage(response);
    }
  }
}

String callGeminiAPI(String prompt) {
  String responseText = "Error: No response";
  
  // Construct JSON payload
  JsonDocument doc;
  JsonObject content = doc["contents"].add<JsonObject>();
  JsonObject part = content["parts"].add<JsonObject>();
  part["text"] = prompt;

  String requestBody;
  serializeJson(doc, requestBody);

  HTTPClient http;
  http.begin(geminiApiUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String payload = http.getString();
    
    // Parse JSON response
    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      responseText = "JSON Parse Error";
    } else {
      if (responseDoc.containsKey("candidates") && responseDoc["candidates"].is<JsonArray>()) {
        responseText = responseDoc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
      } else if (responseDoc.containsKey("error")) {
        responseText = "API Error: " + responseDoc["error"]["message"].as<String>();
      }
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    responseText = "HTTP Error: " + String(httpResponseCode);
  }

  http.end();
  return responseText;
}

void displayMessage(String text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextWrap(true);
  display.println(text);
  display.display();
  Serial.println("Gemini: " + text);
}