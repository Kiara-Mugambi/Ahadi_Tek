
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RTClib.h>
#include <Keypad.h>

MCUFRIEND_kbv tft;

#define TFT_RST 8 // Reset Pin

RTC_DS3231 rtc;

// Define the keypad
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

enum MenuState { HOME, PAY_RENT_MENU, PHONE_NUMBER_MENU, VERIFICATION_MENU };

enum LoadingState { LOADING_PROGRESS, LOADING_DONE };
LoadingState loadingState = LOADING_PROGRESS;

MenuState currentMenu = HOME;

char phoneNumber[11]; // Assuming a 10-digit phone number plus null terminator
//char phoneNumber[0] = '\0'; // Initialize the phone number as empty
int phoneNumberIndex = 0; // Index to keep track of the current position in phoneNumber
bool isVerificationInProgress = false; // Flag to track if verification is ongoing
bool isVerificationComplete = false;

int progressBarWidth = 0; // Width of the progress bar
int progressBarX = 20;   // X position of the progress bar
int progressBarY = 100;  // Y position of the progress bar
int progressBarSpeed = 2; // Speed of the progress bar movement


void setup() {
  tft.reset(); // Reset the display
  uint16_t ID = tft.readID(); // Read the display ID
  tft.begin(ID); // Initialize the display with the ID
  tft.fillScreen(0); // Clear the screen

  // Set text color and size
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);

  // Set display rotation to landscape
  tft.setRotation(3); // Use 1, 2, or 3 for landscape orientations

  // Initialize RTC
  rtc.begin();

  // Display the loading animation for 2 seconds (2000 milliseconds)
  displayLoadingAnimation(2000);

  // Display the homepage with time, "AHADI_TEK," and "Smart Door Lock"
  displayHomePage();
}

void handlePhoneNumberInputMenu(char key);

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    switch (key) {
      case '#':
        if (currentMenu == HOME) {
          currentMenu = PAY_RENT_MENU;
          displayPayRentMenu();
        } else if (currentMenu == PAY_RENT_MENU) {
          // Handle pay rent menu options
          handlePayRentInputMenu();
        } else if (currentMenu == PHONE_NUMBER_MENU) {
          // Handle phone number input menu
          handlePhoneNumberInputMenu(key);
        } else if (currentMenu == VERIFICATION_MENU) {
          // Handle verification process
          handleVerification();
        }
        break;
      case '*':
        if (currentMenu != HOME) {
          currentMenu = HOME;
          displayHomePage();
        }
        break;
      case '0' ... '9':
        // Handle numeric key presses
        handleNumericKeyPress(key);
        break;
      // Handle other keys and menu states here
    }
  }

  // The rest of your loop function remains unchanged
}

void displayHomePage() {
  // Clear the screen
  tft.fillScreen(0);

  // Display "AHADI_TEK" in the middle of the screen
  int textWidth = 13 * 17; // Estimate text width based on character count
  int textX = (tft.width() - textWidth) / 2;
  int textY = (tft.height() - 8) / 2; // 8 pixels tall for this font size

  tft.setCursor(textX, textY);
  tft.setTextColor(0xF800); // Red text color
  tft.setTextSize(4);
  tft.println("AHADI_TEK");

  // Display "Smart Door Lock" just below "AHADI_TEK"
  textY += 45; // Adjust the Y position for the next text
  tft.setCursor(textX, textY);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.println("Smart Door Lock");
}

void displayLoadingAnimation(uint32_t durationMillis) {
  uint32_t startTime = millis();
  int barWidth = 20;
  int barHeight = 10;
  int spacing = 5;
  int barX = (tft.width() - (4 * barWidth + 3 * spacing)) / 2;
  int barY = tft.height() / 2;

  while (millis() - startTime < durationMillis) {
    for (int i = 0; i < 4; i++) {
      tft.fillRect(barX, barY, barWidth, barHeight, 0x07E0); // Green color (RGB565 format)
      barX += barWidth + spacing;
      delay(500); // Adjust the delay as needed for animation speed
    }
    barX = (tft.width() - (4 * barWidth + 3 * spacing)) / 2;
    tft.fillScreen(0);
  }
}

void displayPayRentMenu() {
  tft.fillScreen(0); // Clear the screen
  tft.setCursor(20, 60);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.println("Pay Rent Menu");
  tft.setCursor(20, 100);
  tft.println("Press * to go back");
  currentMenu = PAY_RENT_MENU;
}

void handlePayRentInputMenu() {
  if (currentMenu == PAY_RENT_MENU) {
    // Implement the functionality for the pay rent menu here
    // You can add code to handle input and perform actions as needed
    // This is a placeholder function
    // To transition to the phone number menu, set currentMenu to PHONE_NUMBER_MENU
    currentMenu = PHONE_NUMBER_MENU;
    displayPhoneNumberInput(phoneNumber);
  }
}

void handlePhoneNumberInputMenu(char key) {
  if (key == '*') {
    currentMenu = HOME;
    displayHomePage();
    // Reset the phone number variables when returning to the home menu
    phoneNumberIndex = 0;
    phoneNumber[0] = '\0';
  } else if (key == '#') {
    // Handle submission of the phone number
    // You can add the code here to send/store the phone number
    // For now, let's simulate verification
    currentMenu = VERIFICATION_MENU;
    displayVerificationAnimation();
  } else if (key == 'D' || key == 'd') {
    // Delete the last digit if the phone number is not empty
    if (phoneNumberIndex > 0) {
      phoneNumberIndex--;
      phoneNumber[phoneNumberIndex] = '\0'; // Null-terminate the string
      displayPhoneNumberInput(phoneNumber);
    }
  } else if (phoneNumberIndex < 10 && isDigit(key)) {
    phoneNumber[phoneNumberIndex] = key;
    phoneNumberIndex++;
    phoneNumber[phoneNumberIndex] = '\0'; // Null-terminate the string
    displayPhoneNumberInput(phoneNumber);
  } else {
    // Handle other key presses in the phone number input menu (if needed)
  }
}


void displayPhoneNumberInput(const char* phoneNumber) {
  tft.fillScreen(0); // Clear the screen
  tft.setCursor(20, 60);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.print("Enter Phone Number: ");
  tft.setCursor(20, 100);
  tft.print(phoneNumber);
  tft.setCursor(20, 140);
  tft.print("Press # to submit or * to go back");
}

void handleNumericKeyPress(char key) {
  if (currentMenu == PHONE_NUMBER_MENU && phoneNumberIndex < 10) {
    phoneNumber[phoneNumberIndex] = key;
    phoneNumberIndex++;
    phoneNumber[phoneNumberIndex] = '\0'; // Null-terminate the string
    displayPhoneNumberInput(phoneNumber);
  }
}
void displayVerificationAnimation() {
  tft.fillScreen(0); // Clear the screen
  tft.setTextColor(0x07E0); // Green text color (RGB565 format)
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.print("Verifying");

  // Implement a moving progress bar animation in green color
  if (loadingState == LOADING_PROGRESS) {
    tft.fillRect(progressBarX, progressBarY, progressBarWidth, 10, 0x07E0); // Clear previous progress
    progressBarWidth += progressBarSpeed;
    if (progressBarWidth > (tft.width() - 2 * progressBarX)) {
      progressBarWidth = tft.width() - 2 * progressBarX;
      loadingState = LOADING_DONE;
    }
    tft.fillRect(progressBarX, progressBarY, progressBarWidth, 10, 0x07E0); // Draw new progress
  } else {
    tft.setCursor(progressBarX, progressBarY + 20);
    tft.print("Verification Complete");
  }
}

void handleVerification() {
  // Implement your verification logic here
  // For demonstration, we'll simulate a delay (e.g., 5 seconds)
  delay(5000);
  // Once verification is complete, you can proceed to the next menu or action
  // For now, let's return to the home menu
  currentMenu = HOME;
  displayHomePage();
  isVerificationInProgress = false; // Reset the verification flag
}
