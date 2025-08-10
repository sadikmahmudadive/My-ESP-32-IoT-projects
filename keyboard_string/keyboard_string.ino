#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Keypad config
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Pin assignments
byte rowPins[ROWS] = {13, 12, 14, 27};       // Row 0–3
byte colPins[COLS] = {26, 25, 19, 32};       // Col 0–3, changed col[2] to GPIO 19

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputString = "";

void setup() {
  Serial.begin(9600);
  
  // OLED initialization
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found. Check wiring!"));
    while (true); // halt
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Keypad + OLED Ready");
  display.display();

  Serial.println("Type and press '#' to submit, 'C' to clear:");
  Serial.print("Input: ");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '#') {
      // Show on serial
      Serial.println();
      Serial.print("Submitted String: ");
      Serial.println(inputString);
      Serial.print("String Length: ");
      Serial.println(inputString.length());

      // Show on OLED
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Submitted:");
      display.println(inputString);
      display.print("Length: ");
      display.println(inputString.length());
      display.display();

      inputString = "";
      Serial.print("Input: ");
    } else if (key == 'C') {
      inputString = "";
      Serial.println();
      Serial.println("Input cleared.");
      Serial.print("Input: ");

      // Clear OLED display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Input cleared.");
      display.display();
    } else {
      inputString += key;
      Serial.print(key);

      // Update OLED in real time
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Input:");
      display.println(inputString);
      display.display();
    }
  }
}
