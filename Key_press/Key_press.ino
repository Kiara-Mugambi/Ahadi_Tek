#include <Keypad.h>

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A11, A10, A9, A8}; // Define your row pins here
byte colPins[COLS] = {A15, A14, A13, A12}; // Define your column pins here

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600); // Initialize Serial Monitor
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
  }
}
