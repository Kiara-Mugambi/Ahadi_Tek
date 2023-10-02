#include <SoftwareSerial.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <Keypad.h>

// Create a SoftwareSerial object for communication with the SIM800L module
SoftwareSerial sim800l(18, 19); // RX, TX pins for SIM800L

// Create an MCUFRIEND_kbv object for the TFT screen
MCUFRIEND_kbv tft;

// Define TFT reset pin
#define TFT_RST 8

// Define keypad configuration
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {A15, A14, A13, A12}; // Row pinouts of the keypad
byte colPins[COLS] = {A11, A10, A9, A8};  // Column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Set the house number
String thisHouse = "A18";

// Create the JSON payload for the house number
String dataHouse = "{\"houseNumber\":\"" + thisHouse + "\"}";
String dataPay;

// Set the payment amount and phone number
int newAmount = 1;

// Flags to track payment status and user input
bool paymentComplete = false;
bool paymentStatus = false;
bool pinEntered = false;

// Define time intervals
unsigned long previousCheckTime = 0;
unsigned long paymentCheckInterval = 10UL * 24UL * 60UL * 60UL * 1000UL; // 10 days in milliseconds
unsigned long pinRequestInterval = 5UL * 1000UL; // 5 seconds in milliseconds

// Flag to track payment completion
bool paymentCompleted = false;

// Define a time interval of five days in milliseconds
const unsigned long fiveDays = 5UL * 24UL * 60UL * 60UL * 1000UL;

// Variable to store the last payment check time
unsigned long lastPaymentCheckTime = 0;

// Stores the received prompt message from the server
String promptMessage = "";

// JSON document for parsing server messages
StaticJsonDocument<256> jsonDocument;

// Server and URI details
String server = "zadasmartdoor.000webhostapp.com";
String uri1 = "/checkPay.php";
String uri2 = "/payReceiver.php";

// String to store the device's IP address
String ipAddress;

// String to store the user's phone number
String phoneNumber = "";

unsigned long currentTime;
// Setup function
void setup() {
  // Start serial communication
  Serial.begin(9600);

  // Initialize the SIM800L module
  sim800l.begin(9600);

  //configure HTTP setting
  configureHTTP();

  //Configure the GPRS settings
  configureGPRS();

  //check for and display the obtained IP address
  if (getIPAddress(ipAddress)){
    Serial.print("IP Address: ");
    Serial.println(ipAddress);
  }else {
    Serial.println("Failed to obtain an IP address.");
  }

  // Initialize the TFT screen
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(3); // Adjust rotation as needed

  // Initialize the SIM800L module
  Serial.println("Initializing SIM800L...");
  delay(2000);
  sendATCommand("AT"); // Check if SIM800L is responsive
  sendATCommand("ATE0"); // Disable echo

  // Display a welcome message on the TFT screen
  tft.fillScreen(0x00FF);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Waiting for payment prompt...");
}

// Main loop
void loop() {
  // Check for incoming messages from the server
  checkServerMessages();

  // Handle keypad input
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      // Initiate payment request with collected phone number
      initiatePaymentRequest(phoneNumber);
      phoneNumber = ""; // Reset the phone number input
      tft.fillScreen(0);
      tft.setCursor(10, 10);
      tft.println("Waiting for payment prompt...");
    } else {
      // Collect and display phone number input
      phoneNumber += key;
      tft.setCursor(10, 70);
      tft.println("Phone Number:");
      tft.setCursor(10, 100);
      tft.println(phoneNumber);
    }
  }

  // Your main code here
  // For example, you can add a simple delay and display some content on the screen
  delay(5000);
  tft.fillScreen(0);
  tft.setCursor(10, 10);
  tft.println("Main Code Executing...");

  //initiate an HTTP GET request
  String httpResponseBody = initiateHTTPGetRequest();
}

// Send an AT command to the SIM800L module
void sendATCommand(String command) {
  sim800l.println(command);
  delay(500);
  String response = "";
  while (sim800l.available()) {
    response += sim800l.readString();
  }
  return response;
}

// Check for messages from the server
void checkServerMessages() {
  while (sim800l.available()) {
    char c = sim800l.read();
    if (c == '\n') {
      // Message received from the server, process it
      processServerMessage(promptMessage);
      promptMessage = ""; // Reset the message buffer
    } else {
      promptMessage += c;
    }
  }
}

// Process a message received from the server
void processServerMessage(String message) {
  // Attempt to parse the received JSON payload
  DeserializationError error = deserializeJson(jsonDocument, message);

  // Check if parsing was successful
  if (!error) {
    // Extract and display specific data from the JSON payload
    const char* paymentPrompt = jsonDocument["prompt"];

    // Display the payment prompt on the TFT screen
    tft.fillScreen(0);
    tft.setCursor(10, 10);
    tft.println("Payment Prompt:");
    tft.setCursor(10, 40);
    tft.println(paymentPrompt);
  } else {
    Serial.println("Failed to parse JSON");
  }
}

// Initiate a payment request with the specified phone number
void initiatePaymentRequest(String phoneNumber) {
  // Replace these placeholders with your actual data
  String amount = "10"; // Amount to be paid

  // Send the payment request via USSD code or SMS (depends on your service provider)
  String paymentCode = "*123*1*1*1*" + phoneNumber + "*" + amount + "#";
  sendATCommand("AT+CMGS=\"" + paymentCode + "\"");

  // Wait for response
  delay(2000);
  sendATCommand("\x1A"); // Send CTRL+Z to end SMS

  Serial.println("Payment request sent.");
}

void configureGPRS(){
  //Set APN (Access Point Name) for your mobile carrier
  sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  sendATCommand("AT+SAPBR=3,1,\"APN\",\"your_apn_here\""); //Replace with your carrier's APN

  //set the username and password if required by your carrier (not always necessary)
  sendATCommand("AT+SAPBR=3,1,\"USER\",\"Username_here\"");
  sendATCommand("AT+SAPBR=3,1,\"PWD\",\"password_here\"");

  //Enable GPRS
  sendATCommand("AT+SAPBR=1,1");

  //Query for the GPRS context status (wait until it is connected)
  while (!checkGPRSStatus()){
    delay(1000); // wait for one second before checking again
  }
}

//Function to check GPRS context status
bool checkGPRSStatus(){
  sendATCommand("AT+SAPBR=2,1");
  while (sim800l.available()){
    String response = sim800l.readStringUntil('\n');
    if (response.indexOf("+SAPBR: 1,1") != -1){
      return true;  //GPRS context is connected
    }
  }
  return false; // GPRS context is not connected
}
bool getIPAddress(String &ipAddress){
  sendATCommand("AT+CIFSR");
  while (sim800l.available()){
    String response = sim800l.readStringUntil('\n');
    if (response.startsWith("IP Address: ")){
      ipAddress = response.substring(12);
      return true;  //IP address obtained successfully
    }
  }
  return false;   //IP address not obtained
}
void configureHTTP(){
  //Setting HTTP bearer profile identifier to 1
  sendATCommand("AT+HTTPPARA=\"CID\",1");

  //Setting the URL for my HTTP server
  String serverURL = "http://myserverurl_here.com";
  sendATCommand("AT+HTTPPARA=\"URL\",\"" + serverURL + "\"");

  //setting the URL for the website we want to access
  String websiteURL = "http://mywebsiteURL.com";
  sendATCommand("AT+HTTPPARA=\"URL\",\"" + websiteURL + "\"");

  //setting the HTTP method (GET or POST)
  sendATCommand("AT+HTTPPARA=\REDIR\",1"); //enabling HTTP redirect
  sendATCommand("AT+HTTPPARA=\"UA\",\"Mozilla/5.0\"");  //user agent
  sendATCommand("AT+HTTPPARA=\"TIMEOUT\",60"); //HTTP timeout in seconds
}
//function to initiate an HTTP GET request

void initiateHTTPGetRequest(){
  String response;

  //Start an HTTP session
  sendATCommand("AT+HTTPPARA=\"CID\",1");

  //Initialize the HTTP session with the URL
  sendATCommand("AT+HTTPPARA=\"URL\",\"mywebsiteurl.com\"");

  //set the HTTP method to GET
  sendATCommand("AT+HTTPACTION=0");

  //waiting for the HTTP GET request to complete(Might need a timeout mechanism)
  delay(5000);

  //check the HTTP response code
  String responseCode = sendATCommand("AT+HTTPREAD");
  if (responseCode.indexOf("+HTTPREAD: 0,") != -1){
    //HTTP GET request was successful
    Serial.println("HTTP GET request successful!");

    //Extracting and displaying the received data(webpage content)
    int contentLength = responseCode.substring(responseCode.indexOf("+HTTPREAD: 0,") + 14).toInt();
    response = sendATCommand("AT+HTTPREAD=0," + String(contentLength));
    

  }else{
    //HTTP GET request failed
    Serial.println("HTTP GET request failed.");
    response = ""; //Return an empty string to indicate failure
  }
  //Close the HTTP session
  sendATCommand("AT+HTTPTERM");

  return response;
}