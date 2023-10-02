#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Arduino.h>
#include <ArduinoJson.h>  // Add ArduinoJson library

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

#define espSerial Serial1

String server = "zadasmartdoor.000webhostapp.com";
String uri1 = "/checkPay.php";
String uri2 = "/payReceiver.php";
//String uri3 = "/HouseStatus.json";

String ipAddress;

const int solenoid = 9;

char keyT;

char customKey;

const int rs = 12, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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
//String ssid = "Robott";
//String password = "12345679";
//String ssid = "Mr. Robott";
//String password = "Tesla@443";

/**
String getRequest = "GET " + uri3 + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: keep-alive\r\n\r\n";
String getRequestLength = String(getRequest.length());

**/
String response;
String wf = "";
unsigned long currentTime;

void setup() {
  lcd.begin(16, 2);
  lcd.print("Smart Door");
  loading("Loading");
  espSerial.begin(115200);
  Serial.begin(9600);
  //lockSolenoid();
  pinMode(solenoid, OUTPUT);
  delay(1000);
  lcd.clear();
  lcd.print("Checking WiFi");
  lcd.setCursor(0, 2);
  lcd.print("connection...");
  if (!isWifiConnected()) {
    connectWifi();
  }
}

bool isWifiConnected() {
  espSerial.println("AT+CIPSTATUS");
  delay(1000);

  while (espSerial.available()) {
    String wf = espSerial.readString();
    if (wf.indexOf("STATUS:2") != -1 || wf.indexOf("STATUS:3") != -1) {
      // Wi-Fi is connected or connecting
      return true;
    }
  }

  return false;
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
    lcd.clear();
    lcd.print("checking payment");
    checkPayment();
  }
  processPayment();

}

void processPayment() {

  paymentStatus = checkPaymentStatus(); 

  if (paymentStatus) {
    Serial.println("Thank you");
    lcd.clear();
    lcd.print("Payment Cleared!");
    delay(2000);
    lcd.clear();
    checkPin();
  } else {
    Serial.println("Pay now!");
    lcd.clear();
    lcd.print("Rent is due");
    //lcd.setCursor(0, 2);
    //lcd.print("Pay now!");
    delay(2000);
    paymentMenu();

  }
}


void connectWifi() {
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  espSerial.println(cmd);
  if (espSerial.find("OK")) {
    lcd.clear();
    lcd.print("Connected! ");
    delay(2000);
    lcd.clear();
  } else {
    lcd.clear();
    lcd.print("Connecting to");  // Print the first part of the text
    lcd.setCursor(0, 1);          // Move to the second line
    lcd.print("wifi..");
    delay(1500);
    lcd.clear();
    connectWifi();
  }
}


void checkPin() {
  String pin = "1234";
  String input = "";

  lcd.clear();
  lcd.print("Please enter PIN:");
  while (true) {
    char key = customKeypad.getKey();
    if (key) {
      if (key == '#') {
        break;
      }
      input += key;
      lcd.setCursor(input.length() - 1, 1); // Move the cursor to the next position
      lcd.print("*");
    }
  }

  if (input == pin) {
    lcd.clear();
    lcd.print("Pin Accepted.");
    unlockSolenoid();
    loading(".");
    delay(3000);
    lcd.clear();
    lockSolenoid();
    lcd.print("Locked!!");
    lcd.setCursor(0, 1);
    lcd.print("Press # to unlock!");
    while (true) {
      char keypad = customKeypad.getKey();
      if (keypad == '#'){
        checkPin();
        break;  // Exit the loop after calling checkPin()
      }
    }
  } else {
    lcd.clear();
    lcd.print("Incorrect PIN.");
    lcd.setCursor(0, 1);
    lcd.print("Please try again.");
    delay(2000);
    checkPin();
  }
}

void loading(char msg[]) {
  lcd.setCursor(0, 1);
  lcd.print(msg);

  for (int i = 0; i < 3; i++) {
    delay(1000);
    lcd.print(".");
  }
}


void sendPostRequest() {
  espSerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  if (espSerial.find("OK")) {
    Serial.println("TCP Connection Ready");
  }

  String postRequest = "POST " + uri1 + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Content-Type: application/json\r\n" + "Content-Length: " + String(dataHouse.length()) + "\r\n" + "Connection: keep-alive\r\n\r\n" + dataHouse;
  String postRequestLength = String(postRequest.length());

  espSerial.println("AT+CIPSEND=" + postRequestLength);
  if (espSerial.find(">")) {
    Serial.println("Sending Request...");
    lcd.clear();
    lcd.print("Checking House no.");
    espSerial.print(postRequest);
  }

  if (espSerial.find("SEND OK")) {
    Serial.println("Request Sent");
    espRead();
    delay(2000);
    espSerial.println("AT+CIPCLOSE");
    if (espSerial.find("OK")) {
      Serial.println("TCP Connection Closed");
    }
  }
}

void espRead() {
  String c;
  while (espSerial.available()) {
    c = espSerial.readString();
    Serial.print(c);
    response += c;  // Append to the response string
  }
  return response;
}

bool checkPaymentStatus() {
  // Check if response contains the words "Settled" or "Pay"
  if (response.indexOf("Settled") != -1) {
    return true;
  } else if (response.indexOf("Pay") != -1) {
    return false;
  } else {
    lcd.clear();
    lcd.print("Server Error, Request again!");
    return false;
  }
}



void httppostPay(String dataPay) {
  espSerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");  // Start a TCP connection
  if (espSerial.find("OK")) {
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
  espSerial.print(sendCmd);
  espSerial.println(postRequest.length());
  delay(500);
  if (espSerial.find(">")) {
    Serial.println("Sending...");
    espSerial.print(postRequest);
    if (espSerial.find("SEND OK")) {
      Serial.println("Packet sent");
      delay(1000);
      lcd.clear();
      lcd.print("Sending Prompt");
      while (espSerial.available()) {
        String tmpResp = espSerial.readString();
        Serial.println(tmpResp);
      }
      delay(2000);
      lcd.clear();
      lcd.print("Check your Phone");
      // Close the connection
    }
    espSerial.println("AT+CIPCLOSE");
    delay(30000);
    //processPayment();
    //paymentCompleted = true;
    loop();
  }
}

void paymentMenu() {
  lcd.clear();
  lcd.print("Pay Rent Now!");
  delay(2000);
  lcd.clear();
  lcd.print("Pay via ");
  lcd.setCursor(0, 2);
  lcd.print("  M-PESA  ");
  delay(1500);
  lcd.clear();
  lcd.print("Enter Mpesa number:");
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
          lcd.clear();
          lcd.print("Invalid number!");
          delay(2000);
          lcd.clear();
          break; // Exit the inner loop to return to requesting the phone number again
        }
      } else if (c == '*') {
        if (phoneNumber.length() > 0) {
          phoneNumber.remove(phoneNumber.length() - 1); // Remove the last character from the phone number string
          lcd.setCursor(phoneNumber.length(), 1);
        }
        continue;
      }

      if (c && phoneNumber.length() < 10 && isDigit(c)) {
        phoneNumber += c;
        lcd.setCursor(phoneNumber.length(), 1);
        lcd.print(c);
      }
    }
  }
  
}


void unlockSolenoid() {
  digitalWrite(solenoid, HIGH);
}

void lockSolenoid() {
  digitalWrite(solenoid, LOW);
}


