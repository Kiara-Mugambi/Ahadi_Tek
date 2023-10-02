#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RTClib.h>

MCUFRIEND_kbv tft;

#define TFT_RST 8 // Reset Pin

RTC_DS3231 rtc;

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
  rtc.begin();

  // Display the loading animation for 20 seconds
  displayLoadingAnimation(20000);

  // Display the homepage with time, "AHADI_TEK," and "Smart Door Lock"
  displayHomePage();
}

void loop() {
  // The loop can remain empty or perform other tasks

  // Read the current time from the RTC
  DateTime now = rtc.now();

  // Display the updated time on the screen
  displayTime(now);

  // Print RTC date and time for debugging
  Serial.print("RTC Date: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.println(now.day(), DEC);
  Serial.print("RTC Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  // Delay for a short period (e.g., 1 second)
  delay(1000);
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

void displayTime(DateTime now) {
  // Format and display the time in hours and minutes
  tft.setCursor(20, 60);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(3);
  tft.print(now.hour(), DEC);
  tft.print(':');
  tft.print(now.minute(), DEC);
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
