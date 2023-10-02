#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RTClib.h>
#include <Keypad.h>

#define ROWS 4 // Four rows
#define COLS 4 // Four columns

MCUFRIEND_kbv tft;

#define TFT_RST 8 // Reset Pin
#define RTC_SDA A4 // SDA Pin for RTC
#define RTC_SCL A5 // SCL Pin for RTC

RTC_DS3231 rtc;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A11, A10, A9, A8}; // Analog pins A12 to A15 as row pins
byte colPins[COLS] = {A15, A14, A13, A12}; // Analog pins A8 to A11 as column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

char userInput[11]; // To store user input (maximum 10 digits)

enum MenuState { HOME, PAY_RENT, REGISTER_USER, RESET_PASSWORD, BACK, PAY_RENT_INPUT, OPTIONS_MENU };
MenuState currentMenu = HOME;

void setup() {
  tft.reset(); // Reset the display
  uint16_t ID = tft.readID(); // Read the display ID
  tft.begin(ID); // Initialize the display with the ID
  tft.fillScreen(0); // Clear the screen

  // Set text color and size
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);

  // Set display rotation to landscape
  tft.setRotation(1); // Use 1, 2, or 3 for landscape orientations

  // Initialize RTC
  Wire.begin(); // Initialize I2C communication
  rtc.begin();

  // Initialize keypad
  keypad.setDebounceTime(50); // Debounce time (adjust as needed)

  // Display the homepage with time and labels
  displayHomePage();
}

void loop() {
  char key = keypad.getKey();

  switch (key) {
    case '#':
      currentMenu = HOME; // Reset to the home menu
      displayHomePage();
      break;

    case '*':
      if (currentMenu == HOME) {
        currentMenu = OPTIONS_MENU;
        displayOptionsMenu();
      } else if (currentMenu == OPTIONS_MENU) {
        // Handle the options menu here
        handleOptionsMenu();
      }
      break;

    default:
      if (currentMenu == HOME) {
        handleHomeMenu(key);
      } else if (currentMenu == PAY_RENT) {
        handlePayRentMenu(key);
      } else if (currentMenu == PAY_RENT_INPUT) {
        handlePayRentInputMenu(key);
      }
      // Handle other menus as needed
      break;
  }
}

void displayHomePage() {
  tft.fillScreen(0); // Clear the screen

  // Read the current time from the RTC
  DateTime now = rtc.now();

  // Display the time in hours and minutes and seconds
  int textX = 20;
  int textY = 60;

  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(3);
  tft.setCursor(textX, textY);
  tft.print(now.hour(), DEC);
  tft.print(':');
  tft.print(now.minute(), DEC);
  tft.setTextSize(2);
  tft.print(now.second(), DEC);

  // Display "AHADI_TEK" in the middle of the screen
  int textWidth = 6 * 10; // Estimate text width based on character count
  int textXCenter = (tft.width() - textWidth) / 2;
  int textYCenter = (tft.height() - 8) / 2; // 8 pixels tall for this font size

  tft.setCursor(textXCenter, textYCenter);
  tft.setTextColor(0xF800); // Red text color
  tft.setTextSize(3);
  tft.println("AHADI_TEK");

  // Display "Smart Door Lock" just below "AHADI_TEK"
  textWidth = 13 * 10; // Estimate text width based on character count
  textXCenter = (tft.width() - textWidth) / 2;
  textYCenter += 30; // Shift down by 30 pixels

  tft.setCursor(textXCenter, textYCenter);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.println("Smart Door Lock");
}

void handleHomeMenu(char key) {
  switch (key) {
    case '1':
      displayMessage("Pay Rent Selected");
      delay(1000);
      clearUserInput();
      currentMenu = PAY_RENT_INPUT;
      displayPayRentInput();
      break;

    case '2':
      displayMessage("Register User Selected");
      delay(1000);
      displayHomePage();
      break;

    case '3':
      displayMessage("Reset Password Selected");
      delay(1000);
      displayHomePage();
      break;

    case '4':
      displayMessage("Back Selected");
      delay(1000);
      displayHomePage();
      break;

    default:
      // Handle other keys or do nothing
      break;
  }
}

void displayOptionsMenu() {
  tft.fillScreen(0); // Clear the screen

  // Display the menu options
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);

  int textX = 20;
  int textY = 20;

  tft.setCursor(textX, textY);
  tft.println("1. Pay Rent");

  textY += 30;
  tft.setCursor(textX, textY);
  tft.println("2. Register User");

  textY += 30;
  tft.setCursor(textX, textY);
  tft.println("3. Reset Password");

  textY += 30;
  tft.setCursor(textX, textY);
  tft.println("4. Back");
}

void handleOptionsMenu() {
  switch (key) {
    case '1':
      // Handle "Pay Rent" option
      displayMessage("Pay Rent Selected");
      delay(1000);
      clearUserInput();
      currentMenu = PAY_RENT_INPUT;
      displayPayRentInput();
      break;

    case '2':
      // Handle "Register User" option
      displayMessage("Register User Selected");
      delay(1000);
      displayOptionsMenu();
      break;

    case '3':
      // Handle "Reset Password" option
      displayMessage("Reset Password Selected");
      delay(1000);
      displayOptionsMenu();
      break;

    case '4':
      // Handle "Back" option
      currentMenu = HOME;
      displayHomePage();
      break;

    default:
      // Handle other keys or do nothing
      break;
  }
}


void handleUserInput(char key) {
  int len = strlen(userInput);
  if (len < 10 && isDigit(key)) { // Allow only numeric input for phone number
    userInput[len] = key;
    userInput[len + 1] = '\0'; // Null-terminate the string
    displayPayRentInput();
  }
}


void handlePayRentInputMenu(char key) {
  if (key == '*') {
    // Handle the user input when * is pressed (e.g., send payment prompt to the phone number)
    // handlePayment();
  } else {
    // Collect and display user input
    handleUserInput(key);
  }
}

void displayPayRentInput() {
  tft.fillScreen(0); // Clear the screen

  // Display the input prompt
  int textX = 20;
  int textY = 20;

  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.setCursor(textX, textY);
  tft.println("Enter Phone Number:");

  // Display user input
  textY += 30;
  tft.setCursor(textX, textY);
  tft.println(userInput);
}

void clearUserInput() {
  userInput[0] = '\0';
}

void handlePayRentMenu(char key) {
  // Simulate validation (you should replace this with actual validation logic)
  bool isValidPhoneNumber = (strlen(userInput) == 10);

  if (isValidPhoneNumber) {
    displayMessage("Valid Phone Number");
    delay(1000);

    // Simulate sending an SMS prompt (replace this with actual SMS sending)
    // sendPaymentPrompt(userInput);
    displayMessage("Sending Payment Prompt...");
    delay(2000);

    // Clear the input and return to the home page
    clearUserInput();
    currentMenu = HOME;
    displayHomePage();
  } else {
    displayMessage("Invalid Phone Number");
    delay(1000);

    // Clear the input and return to the home page
    clearUserInput();
    currentMenu = HOME;
    displayHomePage();
  }
}

void displayMessage(const char* message) {
  tft.fillScreen(0); // Clear the screen

  // Display the message in the center of the screen
  int textWidth = strlen(message) * 10; // Estimate text width based on character count
  int textX = (tft.width() - textWidth) / 2;
  int textY = (tft.height() - 8) / 2; // 8 pixels tall for this font size

  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.setCursor(textX, textY);
  tft.println(message);
}
