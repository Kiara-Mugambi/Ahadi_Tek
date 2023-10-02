#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <Keypad.h>
#include <ArduinoJson.h>

SoftwareSerial sim800lSerial(7, 8); // RX, TX for SIM800L

MCUFRIEND_kbv tft;

// Set the house number
String thisHouse = "A18";

// Create the JSON payload
String dataHouse = "{\"houseNumber\":\"" + thisHouse + "\"}";
String dataPay;

// Set the amount and phone number
int newAmount = 1;

bool paymentComplete = false;
bool paymentStatus = false;
bool pinEntered = false;
unsigned long previousCheckTime = 0;
unsigned long paymentCheckInterval = 10 * 24 * 60 * 60 * 1000;  // 10 days in milliseconds
unsigned long pinRequestInterval = 5 * 1000;  // 5 seconds in milliseconds
bool paymentCompleted = false;

const unsigned long fiveDays = 5 * 24 * 60 * 60 * 1000;  // 5 days in milliseconds

unsigned long lastPaymentCheckTime = 0;  // Variable to store the last payment check time

#define solenoidPin 9
char keyT;
char customKey;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[ROWS] = {A7, A6, A5, A4};
byte colPins[COLS] = {A3, A2, A1, A0};

Keypad customKeypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String ssid = "VOLKS ELEVATOR";
String password = "@Volks2021";
String server = "zadasmartdoor.000webhostapp.com";
String uri1 = "/checkPay.php";
String uri2 = "/payReceiver.php";

String ipAddress;

void setup() {
  Serial.begin(9600);
  sim800lSerial.begin(9600); // Initialize SIM800L serial communication
  pinMode(solenoidPin, OUTPUT);

  // Initialize TFT display
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // Lock the solenoid initially
  lockSolenoid();

  delay(1000);
  if (!isWifiConnected()) {
    connectWifi();
  }
}

bool isWifiConnected() {
  // Add your WiFi check logic here
  // Return true if WiFi is connected, false otherwise
}

void checkPayment() {
  sendPostRequest();
  paymentStatus = checkPaymentStatus();
  lastPaymentCheckTime = millis();
}

void loop() {
  currentTime = millis();
  // Check if it's been 5 days since the last check or if paymentStatus is false
  if (currentTime - lastPaymentCheckTime >= fiveDays || !paymentStatus) {
    checkPayment();
  }
  processPayment();
}

void sendPostRequest() {
  sim800lSerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  if (waitForResponse("OK")) {
    Serial.println("TCP Connection Ready");
  }

  String postRequest = "POST " + uri1 + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + String(dataHouse.length()) + "\r\n" +
                       "Connection: keep-alive\r\n\r\n" + dataHouse;
  String postRequestLength = String(postRequest.length());

  sim800lSerial.println("AT+CIPSEND=" + postRequestLength);
  if (waitForResponse(">")) {
    Serial.println("Sending Request...");
    sim800lSerial.print(postRequest);
  }

  if (waitForResponse("SEND OK")) {
    Serial.println("Request Sent");
    delay(2000);
    sim800lSerial.println("AT+CIPCLOSE");
    if (waitForResponse("OK")) {
      Serial.println("TCP Connection Closed");
    }
  }
}

bool waitForResponse(const char* expectedResponse, unsigned long timeout = 5000) {
  unsigned long startTime = millis();
  String response;

  while (millis() - startTime < timeout) {
    if (sim800lSerial.available()) {
      char c = sim800lSerial.read();
      response += c;

      if (response.indexOf(expectedResponse) != -1) {
        return true;
      }
    }
  }

  return false;
}

void httppostPay(String dataPay) {
  sim800lSerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  if (waitForResponse("OK")) {
    Serial.println("TCP Connection Ready");
  }

  String postRequest = "POST " + uri2 + " HTTP/1.0\r\n" +
                       "Host: " + server + "\r\n" +
                       "Accept: */*\r\n" +
                       "Content-Length: " + dataPay.length() + "\r\n" +
                       "Content-Type: application/json\r\n" +
                       "\r\n" + dataPay;
  String sendCmd = "AT+CIPSEND=";
  sim800lSerial.print(sendCmd);
  sim800lSerial.println(postRequest.length());
  delay(500);
  if (waitForResponse(">")) {
    Serial.println("Sending...");
    sim800lSerial.print(postRequest);
    if (waitForResponse("SEND OK")) {
      Serial.println("Packet sent");
      delay(1000);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Check your Phone");
    }
    sim800lSerial.println("AT+CIPCLOSE");
    delay(30000);
    loop();
  }
}

void processPayment() {
  paymentStatus = checkPaymentStatus();

  if (paymentStatus) {
    Serial.println("Thank you");
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("Payment Cleared!");
    delay(2000);
    tft.fillScreen(TFT_BLACK);
    checkPin();
  } else {
    Serial.println("Pay now!");
    tft.fillScreen(TFT_BLACK);
    tft.println("Rent is due");
    delay(2000);
    paymentMenu();
  }
}

void checkPin() {
  String pin = "1234";
  String input = "";

  tft.println("Please enter PIN:");
  while (true) {
    char key = customKeypad.getKey();
    if (key) {
      if (key == '#') {
        break;
      }
      input += key;
      tft.print("*");
    }
  }

  if (input == pin) {
    tft.fillScreen(TFT_BLACK);
    tft.println("Pin Accepted.");
    unlockSolenoid();
    loading(".");
    delay(3000);
    tft.fillScreen(TFT_BLACK);
    lockSolenoid();
    tft.println("Locked!!");
    tft.setCursor(0, 1);
    tft.println("Press # to unlock!");
    while (true) {
      char keypad = customKeypad.getKey();
      if (keypad == '#'){
        checkPin();
        break;  // Exit the loop after calling checkPin()
      }
    }
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.println("Incorrect PIN.");
    tft.setCursor(0, 1);
    tft.println("Please try again.");
    delay(2000);
    checkPin();
  }
}

void loading(char msg[]) {
  tft.setCursor(0, 0);
  tft.println(msg);

  for (int i = 0; i < 3; i++) {
    delay(1000);
    tft.println(".");
  }
}

void unlockSolenoid() {
  digitalWrite(solenoidPin, HIGH);
}

void lockSolenoid() {
  digitalWrite(solenoidPin, LOW);
}

bool checkPaymentStatus() {
  // Check if response contains the words "Settled" or "Pay"
  if (response.indexOf("Settled") != -1) {
    return true;
  } else if (response.indexOf("Pay") != -1) {
    return false;
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.println("Server Error, Request again!");
    return false;
  }
}

void httppostPay(String dataPay) {
  sim800lSerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");  // Start a TCP connection
  if (waitForResponse("OK")) {
    Serial.println("TCP connection ready");
  }
  delay(100);
  String postRequest =
    "POST " + uri2 + " HTTP/1.0\r\n" +
    "Host: " + server + "\r\n" +
    "Accept: */*\r\n" +
    "Content-Length: " + dataPay.length() + "\r\n" +
    "Content-Type: application/json\r\n" +
    "\r\n" + dataPay;
  String sendCmd = "AT+CIPSEND=";  // Determine the number of characters to be sent
  sim800lSerial.print(sendCmd);
  sim800lSerial.println(postRequest.length());
  delay(500);
  if (waitForResponse(">")) {
    Serial.println("Sending...");
    sim800lSerial.print(postRequest);
    if (waitForResponse("SEND OK")) {
      Serial.println("Packet sent");
      delay(1000);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Check your Phone");
    }
    sim800lSerial.println("AT+CIPCLOSE");
    delay(30000);
    loop();
  }
}

void paymentMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.println("Pay Rent Now!");
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  tft.println("Pay via ");
  tft.setCursor(0, 1);
  tft.println("  M-PESA  ");
  delay(1500);
  tft.fillScreen(TFT_BLACK);
  tft.println("Enter Mpesa number:");
  String phoneNumber = "";
  bool isValidNumber = false; // Add a flag variable to track the number validity
  while (!isValidNumber) {
    while (true) {
      char c = customKeypad.getKey();
      if (c == '#') {
        if (phoneNumber.length() == 10) {
          // Create the JSON payload
          dataPay = "{\"amount\":" + String(newAmount) + ",\"phoneNumber\":\"" + phoneNumber + "\"}";
          httppostPay(dataPay);
          // Exit the inner loop to indicate that payment is initiated
          break;
        } else {
          tft.fillScreen(TFT_BLACK);
          tft.println("Invalid number!");
          delay(2000);
          tft.fillScreen(TFT_BLACK);
          break; // Exit the inner loop to return to requesting the phone number again
        }
      } else if (c == '*') {
        if (phoneNumber.length() > 0) {
          phoneNumber.remove(phoneNumber.length() - 1); // Remove the last character from the phone number string
          tft.setCursor(phoneNumber.length(), 1);
        }
        continue;
      }

      if (c && phoneNumber.length() < 10 && isDigit(c)) {
        phoneNumber += c;
        tft.setCursor(phoneNumber.length(), 1);
        tft.print(c);
      }
    }
  }
}
