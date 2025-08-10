#include <Keypad.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Configuration ---
const char CORRECT_PASS[] = "1237";
const uint8_t PASSWORD_LENGTH = 4;
const unsigned long UNLOCK_DURATION_MS = 5000; // Time in milliseconds the door stays unlocked
const unsigned long MSG_DURATION_MS = 2000;    // Time in milliseconds for status messages to show

// --- Hardware Pins ---
const int servoPin = 13;
const byte ROWS = 4;
const byte COLS = 4;
byte rowPins[ROWS] = {32, 33, 25, 26}; // ESP32 pins for keypad rows
byte colPins[COLS] = {17, 16, 4, 15};  // ESP32 pins for keypad columns
// For OLED display: SDA to pin 21, SCL to pin 22

// --- Servo Angles ---
const int LOCK_ANGLE = 10;
const int UNLOCK_ANGLE = 90;

// --- OLED Display Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Keypad Layout ---
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Global Variables ---
Servo myServo;
String inputPass = "";
bool unlocked = false;
unsigned long stateChangeMillis = 0; // Tracks time for auto-lock or message clearing
String statusMsg = "LOCKED";
bool temporaryMsgActive = false;


void setup() {
  Serial.begin(115200);

  // Initialize OLED Display
  Wire.begin(21, 22); // Specify SDA, SCL pins
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Initialize Servo
  myServo.attach(servoPin);
  myServo.write(LOCK_ANGLE); // IMPORTANT: Start in the locked position
  unlocked = false;
  statusMsg = "LOCKED";
  
  Serial.println("System Ready: ESP32 Keypad Door Lock");
  drawScreen();
}

// =================================================================
// LOOP: Runs continuously
// =================================================================
void loop() {
  char key = keypad.getKey();
  if (key) {
    handleKeyPress(key);
  }

  // Check if the door should be auto-locked
  if (unlocked && (millis() - stateChangeMillis >= UNLOCK_DURATION_MS)) {
    lockDoor();
  }

  // Check if a temporary message (like "WRONG PASS") should be cleared
  if (temporaryMsgActive && (millis() - stateChangeMillis >= MSG_DURATION_MS)) {
    temporaryMsgActive = false;
    statusMsg = unlocked ? "UNLOCKED" : "LOCKED"; // Revert to the actual current state
    drawScreen();
  }
}


void handleKeyPress(char key) {
  if (key == '*') { // Clear last digit
    if (inputPass.length() > 0) {
      inputPass.remove(inputPass.length() - 1);
    }
  } else if (key == '#') { // Submit password
    checkPassword();
  } else if (key >= '0' && key <= '9') { // Append digit
    if (inputPass.length() < PASSWORD_LENGTH) {
      inputPass += key;
    }
  }
  // Ignore other keys like A, B, C, D
  
  drawScreen(); // Update the display after every key press
}

/**
 * @brief Compares the entered password with the correct one.
 */
void checkPassword() {
  if (inputPass == String(CORRECT_PASS)) {
    unlockDoor();
  } else {
    showTemporaryMessage(inputPass.length() < PASSWORD_LENGTH ? "TOO SHORT" : "WRONG PASS");
    inputPass = ""; // Clear the wrong password
  }
}

/**
 * @brief Unlocks the door by moving the servo.
 */
void unlockDoor() {
  myServo.write(UNLOCK_ANGLE);
  unlocked = true;
  statusMsg = "UNLOCKED";
  inputPass = "";
  stateChangeMillis = millis(); // Reset timer for auto-lock
  temporaryMsgActive = false;
  Serial.println("Door UNLOCKED");
  drawScreen();
}

/**
 * @brief Locks the door by moving the servo.
 */
void lockDoor() {
  myServo.write(LOCK_ANGLE);
  unlocked = false;
  statusMsg = "LOCKED";
  stateChangeMillis = millis();
  temporaryMsgActive = false;
  Serial.println("Door LOCKED");
  drawScreen();
}

/**
 * @brief Displays a temporary message on the screen (e.g., "WRONG PASS").
 */
void showTemporaryMessage(String msg) {
  statusMsg = msg;
  temporaryMsgActive = true;
  stateChangeMillis = millis(); // Reset timer for message clearing
  Serial.println("Status: " + msg);
  drawScreen();
}

/**
 * @brief Redraws the entire OLED display with current status.
 */
void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,0);
  display.println("ESP32 Keypad Door Lock");
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("Status:");
  display.setCursor(0, 40);
  display.print(statusMsg);

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Input: ");
  String maskedPass = "";
  for (uint8_t i = 0; i < inputPass.length(); i++) {
    maskedPass += '*';
  }
  display.print(maskedPass);

  display.display();
}