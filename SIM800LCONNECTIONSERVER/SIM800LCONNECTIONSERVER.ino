//AT+CSQ command for checking the cellular signal level or rather the signal quality
//AT+CFUN checks for the device full functionality mode
//AT+CGATT=1 for connecting to a data network


/*next commands are for checking or rather for connecting to our network properly*/
//AT+SAPBR=3,1,"CONTYPE","GPRS" for configuring the bearer profile 1
//AT+SAPBR=3,1,"APN","internet" APN for the internet
//AT+SAPBR=1,1 for opening a GPRS context

/*Establishing a web session by setting up a browser-like environment for the sim800*/
//AT+HTTPINIT for initializing the HTTP service
//AT+HTTPPARA="CID",1 for setting parameters for the HTTP session
//AT+HTTPPARA="REDIR",1 for auto redirect

//AT+HTTPPARA="URL" for inputing the website URL that i wish to download
//AT+HTTPACTION=0 tells the sim800 to start downloading the web page

//AT+HTTPREAD for viewing the whole content of the web page through the serial monitor

#include <SoftwareSerial.h>

SoftwareSerial sim800lSerial(18, 19);  // RX, TX pins for SIM800L

void s  etup() {
  Serial.begin(9600);       // Serial monitor
  sim800lSerial.begin(9600); // SIM800L module

  delay(5000); // Give SIM800L some time to initialize

  // Initialize the SIM800L module
  if (sendATCommand("AT")) {
    Serial.println("SIM800L module initialized successfully");
    
    // Check if the module is in full functionality mode
    if (checkFullFunctionalityMode()) {
      Serial.println("Module is in Full Functionality Mode");
      
      // Configure bearer profile 1 and open GPRS context
      if (configureBearerProfile1() && openGPRSContext()) {
        Serial.println("GPRS Context opened successfully");
        
        // Establish a web session
        if (establishWebSession()) {
          Serial.println("Web session established successfully");
        } else {
          Serial.println("Failed to establish web session");
        }
      } else {
        Serial.println("Failed to open GPRS Context");
      }
    } else {
      Serial.println("Module is NOT in Full Functionality Mode");
    }

    // Check signal quality
    getSignalQuality();
  } else {
    Serial.println("Failed to initialize SIM800L module");
  }
}

void loop() {
  // Your main code here
}

String sendATCommand(String command) {
  sim800lSerial.println(command);
  delay(500);
  String response = "";
  
  while (sim800lSerial.available()) {
    response = sim800lSerial.readStringUntil('\n');
    Serial.println(response); // Print response to Serial Monitor
  }
  
  return response;
}

bool checkFullFunctionalityMode() {
  String response = sendATCommand("AT+CFUN?");
  return response.indexOf("+CFUN: 1") != -1;
}

bool configureBearerProfile1() {
  // Configure bearer profile 1 (replace with your values)
  sendATCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendATCommand("AT+SAPBR=3,1,\"APN\",\"\"");
  sendATCommand("AT+SAPBR=3,1,\"USER\",\"\"");
  sendATCommand("AT+SAPBR=3,1,\"PWD\",\"\"");
  
  // Open bearer profile 1
  String response = sendATCommand("AT+SAPBR=1,1");
  return response.indexOf("1,1,\"") != -1;
}

bool openGPRSContext() {
  // Open a GPRS context
  sendATCommand("AT+CIICR");
  
  // Check if the GPRS context is opened
  String response = sendATCommand("AT+CIFSR");
  return response.indexOf('.') != -1;
}

bool establishWebSession() {
  // Start a TCP/IP connection (replace "your_server" and "your_port" with your server's details)
  sendATCommand("AT+CIPSTART=\"TCP\",\"zadasmartdoor.000webhostapp.com\",\"80\"");
  
  // Check if the connection is established
  String response = sendATCommand("AT+CIPSTATUS");
  return response.indexOf("CONNECT OK") != -1;
}

void getSignalQuality() {
  String response = sendATCommand("AT+CSQ");
  if (response.indexOf("+CSQ:") != -1) {
    int commaIndex = response.indexOf(",");
    if (commaIndex != -1) {
      String signalQuality = response.substring(commaIndex + 1, response.indexOf("\n"));
      Serial.println("Signal Quality: " + signalQuality);
    }
  }
}
