#include <SoftwareSerial.h>

// Define the SoftwareSerial object for SIM800L communication
SoftwareSerial sim800l(19,18); // RX, TX

void setup() {
  // Start communication with SIM800L
  sim800l.begin(9600);

  // Initialize Serial Monitor for debugging
  Serial.begin(9600);
  Serial.println("SIM800L HTTP Client");

  // Wait for SIM800L to register on the network
  while (!sim800l.available()) {
    delay(10000);
    Serial.println("Waiting for SIM800L to register on the network...");
  }

  // Enable GPRS
  sim800l.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(1000);
  sim800l.println("AT+SAPBR=3,1,\"APN\",\"safaricom\"");
  delay(1000);
  sim800l.println("AT+SAPBR=1,1");
  delay(1000);
}

void loop() {
  // Send an HTTP GET request to a server
  sim800l.println("AT+HTTPINIT");
  delay(1000);
  sim800l.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);
  sim800l.println("AT+HTTPPARA=\"URL\",\"http://zadasmartdoor.000webhostapp.com/houses.php\"");
  delay(1000);
  sim800l.println("AT+HTTPACTION=0");
  delay(5000); // Wait for the server to respond

  // Read and print the server's response
  sim800l.println("AT+HTTPREAD");
  delay(1000);

  while (sim800l.available()) {
    char c = sim800l.read();
    Serial.print(c); // Print the server's response to Serial Monitor
  }

  sim800l.println("AT+HTTPTERM");
  delay(1000);

  // You can adjust the timing and add error handling as needed
}
