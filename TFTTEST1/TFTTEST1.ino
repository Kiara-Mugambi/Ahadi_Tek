#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <Keypad.h>

MCUFRIEND_kbv tft;

// Define TFT display pins
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RST A4

// Define colors - These may vary depending on your display
#define TFT_BLACK   0x0000
#define TFT_WHITE   0xFFFF
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_BLUE    0x001F

// Define the keypad pins and layout
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
char customKey;

// Replace these with your actual row and column pins
byte rowPins[ROWS] = {A11, A10, A9, A8};
byte colPins[COLS] = {A15, A14, A13, A12};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Define the solenoid lock pin
const int solenoidPin = 12;

bool doorLocked = true;

void setup() {
  // Initialize the TFT display
  tft.reset();
  uint16_t identifier = tft.readID();
  if (identifier == 0x9341) {
    tft.begin(identifier);
  } else {
    tft.begin(identifier);
  }

  // Initialize the solenoid lock pin
  pinMode(solenoidPin, OUTPUT);
  digitalWrite(solenoidPin, LOW); // Ensure the solenoid is initially locked

  // Display the welcome message and homepage
  displayHomepage();
}

void loop() {
  // Read the keypad
  customKey = keypad.getKey();

  if (customKey == '#') {
    if (doorLocked) {
      unlockDoor();
      tft.setCursor(20, 150);
      tft.print("Door Unlocked");
    } else {
      tft.setCursor(20, 150);
      tft.print("Door Already Unlocked");
    }

    delay(2000);
    tft.setCursor(20, 150);
    tft.fillRect(20, 150, 200, 30, TFT_BLACK); // Clear the status message
  } else if (customKey != NO_KEY) {
    checkPassword(customKey);
  }
  Serial.begin(9600); // Add this line in setup

// Add this line in the loop to check keypad inputs
if (customKey != NO_KEY) {
  Serial.println(customKey); // Debug line to check keypad input
  checkPassword(customKey);
}

}

void unlockDoor() {
  digitalWrite(solenoidPin, HIGH); // Activate the solenoid to unlock the door
  doorLocked = false;
}

void lockDoor() {
  digitalWrite(solenoidPin, LOW); // Deactivate the solenoid to lock the door again
  doorLocked = true;
}

void checkPassword(char key) {
  static char password[5] = "1234"; // Define the password
  static byte index = 0;

  if (key == password[index]) {
    index++;
    
    if (index == 4) {
      unlockDoor();
      tft.setCursor(20, 150);
      tft.print("Door Unlocked");
      delay(2000);
      tft.setCursor(20, 150);
      tft.fillRect(20, 150, 200, 30, TFT_BLACK); // Clear the status message
      index = 0;
    }
  } else {
    index = 0; // Reset the index if a wrong key is pressed
  }
}

void displayHomepage() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.println("AHADI_TEK");
  tft.setTextSize(1);
  tft.setCursor(40, 80);
  tft.println("Smart Door Lock");

  while (true) {
    // Display the status of whether the door is locked or unlocked
    tft.setTextSize(2);
    tft.setCursor(20, 120);
    tft.print("Locked: ");
    tft.print(doorLocked ? "Yes" : "No");
    delay(1000); // Update the status every second
  }
}
