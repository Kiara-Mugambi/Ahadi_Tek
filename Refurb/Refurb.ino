#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RTClib.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

bool paymentComplete = false;
bool paymentStatus = false;
bool pinEntered = false;
unsigned long previousCheckTime =0;
unsigned long paymentCheckInterval =10*24*60*60*1000;
unsigned long pinRequestInterval =5*1000;
bool paymentCompleted =false;

const unsigned long fiveDays =5*24*60*60*1000;

unsigned long lastPaymentCheckTime =0;
unsigned long currentTime;

int newAmount =1;

const int solenoid=28;
SoftwareSerial sim800lSerial(19,18);
MCUFRIEND_kbv tft;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};

byte rowPins[ROWS] ={A7, A6, A5, A4};
byte colPins[COLS] ={A3, A2, A1, A0};

Keypad customKeypad(makeKeymap(keys),rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  sim800lSerial.begin(9600);
  pinMode(solenoid, OUTPUT);
  delay(1000);
  
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.fillScreen(0);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  tft.setRotation(3);
  //rtc.begin();

  //displayHomePage();

}

void checkPayment(){
  sendPostRequest();
  paymentStatus = checkPaymentStatus();
  lastPaymentCheckTime = millis();
}

void loop() {
  currentTime = millis();

  if (currentTime - lastPaymentCheckTime >=fiveDays || !paymentStatus)
  {
    int textWidth = 13*17;
    int textX = (tft.width() - textWidth)/2;
    int textY = (tft.height() -8)/2;

    tft.setCursor(textX, textY);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(4);
    tft.println("Checking Payment");
    checkPayment();

  }
  processPayment();

  // put your main code here, to run repeatedly:

}

void processPayment(){
  paymentStatus = checkPaymentStatus();

  if (paymentStatus){
    Serial.println("Thank You");
    tft.fillScreen(0);

    int textWidth =13*17;
    int textX =(tft.width()- textWidth)/2;
    int textY =(tft.height()- 8)/2;

    tft.setCursor(textX, textY);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(4);
    tft.println("Payment Cleared!");
    delay(1000);
    tft.fillScreen(0);
    checkPin();
  }
  else{
    Serial.println("Pay Now");
    tft.fillScreen(0);


    int textWidth =13*17;
    int textX =(tft.width() - textWidth)/2;
    int textY =(tft.height()- 8)/2;

    tft.setCursor(textX, textY);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(3);
    tft.println("Rent is Due");
    delay(10000);
    paymentMenu();
  }
}

void checkPin(){
  String pin="1234";
  String input ="";

  tft.fillScreen(0);

  int textWidth =13*17;
  int textX =(tft.width()- textWidth)/2;
  int textY =(tft.height()-8)/2;

  tft.setCursor(textX, textY);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(3);
  tft.println("Enter Pin: ");
  while (true){
    char key = customKeypad.getKey();
    if(key){
      if(key == "#"){
        break;
      }
      input +=key;
      textY +=20;

      tft.setCursor(textX, textY);
      tft.setTextColor(0xFFFF);
      tft.setTextSize(3);
      tft.println("*");

    }
  }

  if (input == pin){
    tft.fillScreen(0);
    tft.println("Pin Accepted.");
    unlockSolenoid();
    loading(".");
    delay(3000);
    tft.fillScreen(0);
    lockSolenoid();
    tft.println("Locked!!");

    int textWidth =8*11;
    int textX= (tft.width()- textWidth)/2;
    int textY =(tft.height()-8)/2;

    tft.setTextColor(0xFFFF);
    tft.setTextSize(4);
    tft.println("Press # to Unlock!");
    while (true){
      char keypad = customKeypad.getKey();
      if (keypad == "#"){
        checkPin();
        break;
      }
    }

  }
  else{
    tft.fillScreen(0);
    
    int textWidth = 13*17;
    int textX = (tft.width()- textWidth)/2;
    int textY = (tft.height()- 8)/2;

    tft.setTextColor(0xFFFF);
    tft.setTextSize(4);
    tft.println("Incorrect Pin.");

    textY +=20;
    textY =(tft.height()- 8)/2;

    tft.setTextColor(0xFFFF);
    tft.setTextSize(4);
    tft.println("Please Try Again.");
    delay(2000);
    checkPin();
  }
}

void loading(char msg[]){

  int textY =(tft.height() - 8)/2;

  textY +=20;

  tft.setTextColor(0xFFFF);
  tft.println(msg);

  for (int i=0; i<3; i++){
    delay(1000);
    tft.println(".");
  }
}

void sendPostRequest(){

}

bool checkPaymentStatus(){
  /*if (response.indexOf("Settled")! =-1){
    return true;
  }
  else if (response.indexOf("Pay")! =-1){
    return false;
  }else{
    tft.fillScreen(0);
    int textWidth =13*17;
    int textX =(tft.width()- textWidth)/2;
    int textY =(tft.height()- 8)/2;

    tft.setTextColor(0xFFFF);
    tft.setTextSize(4);
    tft.println("Server Error, Request Again!");
    return false;
  }*/
}
/*void httppostPay(string dataPay){
  loop();
}
*/
void paymentMenu(){

  tft.fillScreen(0);
  int textWidth = 13*17;
  int textX =(tft.width()- textWidth)/2;
  int textY =(tft.height()- 8)/2;

  tft.setTextColor(0xFFFF);
  tft.setTextSize(4);
  tft.println("Pay Rent Now!");
  delay(2000);
  tft.fillScreen(0);
  tft.println("Pay Via");
  
  textY =(tft.height()- 8)/2;
  textY +=20;
  
  tft.println("   M-PESA   ");
  delay(1500);
  tft.fillScreen(0);

  textWidth = 13*17;
  textX =(tft.width()- textWidth)/2;
  textY =(tft.height()- 8)/2;

  tft.setTextColor(0xF800);
  tft.setTextSize(4);
  tft.println("Enter M-Pesa Number:");
  String phoneNumber ="";
  bool isValidNumber =false;
  while (!isValidNumber){
    while (true){
      char c = customKeypad.getKey();
      if (c == "#"){
        if (phoneNumber.length()== 10){
          String dataPay = "{\"amount\":" + String(newAmount) + ",\"phoneNumber\":\"" + phoneNumber + "\"}";
          //httppostPay(dataPay);

          break;
        }
        else{
          tft.fillScreen(0);
          tft.println("Invalid Number!");
          delay(2000);
          tft.fillScreen(0);
          break;
        }
      } else if (c == "*"){
        if (phoneNumber.length() > 0){
          phoneNumber.remove(phoneNumber.length() -1);
          tft.setCursor(phoneNumber.length(), 1);
        }
        continue;
      }
      if (c && phoneNumber.length() < 10 && isDigit(c)){
        phoneNumber +=c;
        tft.setCursor(phoneNumber.length(), 1);
        tft.println(c);
      }
    }
  }

}

void unlockSolenoid(){
  digitalWrite(solenoid, HIGH);
}

void lockSolenoid(){
  digitalWrite(solenoid, LOW);
}