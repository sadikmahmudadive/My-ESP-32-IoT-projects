#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Use safe GPIOs on ESP32
byte rowPins[ROWS] = {13, 12, 14, 27};  // Adjust to your wiring
byte colPins[COLS] = {26, 25, 23, 32};  // Adjust to your wiring

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);  // Changed from 115200 to 9600
  Serial.println("Keypad Ready. Press a key:");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == 'C') {
      Serial.println("\n--- Screen Cleared ---\n");
      for (int i = 0; i < 20; i++) Serial.println();
    } else {
      Serial.print("Key Pressed: ");
      Serial.println(key);
    }
  }
}
