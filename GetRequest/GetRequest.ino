#include <SoftwareSerial.h>

SoftwareSerial sim800lSerial(18, 19);  // RX, TX pins for SIM800L

void setup() {
  Serial.begin(9600);       // Serial monitor
  sim800lSerial.begin(9600); // SIM800L module

  delay(1000); // Give SIM800L some time to initialize

  // Initialize the SIM800L module
  sendATCommand("AT");
  delay(1000);

  // Check if the module is in full functionality mode
  if (checkFullFunctionalityMode()) {
    Serial.println("Module is in Full Functionality Mode");
    
    // Configure bearer profile 1 and open GPRS context
    if (configureBearerProfile1() && openGPRSContext()) {
      Serial.println("GPRS Context opened successfully");
      
      // Establish a web session
      if (establishWebSession()) {
        Serial.println("Web session established successfully");

        // Send an HTTP GET request (replace "your_server" and "your_endpoint" with your server details)
        String server = "your_server";
        String endpoint = "your_endpoint";
        if (sendHttpGetRequest(server, endpoint)) {
          Serial.println("HTTP GET request sent successfully");
          
          

          
          // Read and print the HTTP response
          String response = readHttpResponse();
          Serial.println("HTTP Response:");
          Serial.println(response);
        } else {
          Serial.println("Failed to send HTTP GET request");
        }
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
}

void loop() {
  // Your main code here
}

void sendATCommand(String command) {
  sim800lSerial.println(command);
  delay(500);
  while (sim800lSerial.available()) {
    Serial.write(sim800lSerial.read());
  }
}

bool checkFullFunctionalityMode() {
  sendATCommand("AT+CFUN?");
  while (!sim800lSerial.available()) {
    // Wait for response
  }

  String response = sim800lSerial.readStringUntil('\n');
  return response.indexOf("+CFUN: 1") != -1;
}

bool configureBearerProfile1() {
  // Configure bearer profile 1 (replace with your values)
  sendATCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendATCommand("AT+SAPBR=3,1,\"APN\",\"your_apn\"");
  sendATCommand("AT+SAPBR=3,1,\"USER\",\"your_username\"");
  sendATCommand("AT+SAPBR=3,1,\"PWD\",\"your_password\"");
  
  // Open bearer profile 1
  sendATCommand("AT+SAPBR=1,1");
  
  // Check if bearer profile 1 is configured
  delay(1000);
  sendATCommand("AT+SAPBR=2,1");

  while (!sim800lSerial.available()) {
    // Wait for response
  }

  String response = sim800lSerial.readStringUntil('\n');
  return response.indexOf("1,1,\"") != -1;
}

bool openGPRSContext() {
  // Open a GPRS context
  sendATCommand("AT+CIICR");
  
  // Check if the GPRS context is opened
  delay(1000);
  sendATCommand("AT+CIFSR");

  while (!sim800lSerial.available()) {
    // Wait for response
  }

  String response = sim800lSerial.readStringUntil('\n');
  return response.indexOf('.') != -1;
}

bool establishWebSession() {
  // Start a TCP/IP connection (replace "your_server" and "your_port" with your server's details)
  sendATCommand("AT+CIPSTART=\"TCP\",\"your_server\",\"80\"");
  
  // Check if the connection is established
  while (!sim800lSerial.available()) {
    // Wait for response
  }

  String response = sim800lSerial.readStringUntil('\n');
  return response.indexOf("CONNECT OK") != -1;
}

bool sendHttpGetRequest(String server, String endpoint) {
  // Build and send an HTTP GET request
  String request = "GET " + endpoint + " HTTP/1.1\r\n";
  request += "Host: " + server + "\r\n";
  request += "Connection: close\r\n\r\n";
  
  sendATCommand("AT+CIPSEND");
  
  // Check if ">" prompt is received
  while (!sim800lSerial.available() || sim800lSerial.read() != '>');
  
  // Send the HTTP GET request
  sim800lSerial.println(request);
  
  // Wait for the response header
  delay(1000);
  
  return true;
}

String readHttpResponse() {
  String response = "";
  
  // Read the HTTP response
  while (sim800lSerial.available()) {
    response += sim800lSerial.readString();
  }
  
  return response;
}

void getSignalQuality() {
  sendATCommand("AT+CSQ");

  while (!sim800lSerial.available()) {
    // Wait for response
  }

  String response = sim800lSerial.readStringUntil('\n');
  Serial.println("Signal Quality: " + response);
}
