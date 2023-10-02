#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RTClib.h>
#include <Keypad.h>

#include <SPI.h>             // f.k. for Arduino-1.5.2
#define USE_SDFAT
#include <SdFat.h>           // Use the SdFat library
SdFatSoftSpi<12, 11, 13> SD; //Bit-Bang on the Shield pins

MCUFRIEND_kbv tft;

#define SD_CS     10
#define NAMEMATCH ""         // "" matches any name
//#define NAMEMATCH "tiger"    // tiger.bmp
#define PALETTEDEPTH   8     // support 256-colour Palette

char namebuf[32] = "/";   //BMP files in root directory
//char namebuf[32] = "/bitmaps/";  //BMP directory e.g. files in /bitmaps/*.bmp

File root;
int pathlen;

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

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
unsigned long currentMillis = millis();
unsigned long lastAnimationMillis = 0;
unsigned long bitmapStartTime = 0;
unsigned long bitmapDuration = 2000;

char phoneNumber[11]; // Assuming a 10-digit phone number plus null terminator
int phoneNumberIndex = 0; // Index to keep track of the current position in phoneNumber
bool isVerificationInProgress = false; // Flag to track if verification is ongoing
bool isVerificationComplete = false;
bool isPhoneNumberComplete = false;

int progressBarWidth = 0; // Width of the progress bar
int progressBarX = 20;   // X position of the progress bar
int progressBarY = 100;  // Y position of the progress bar
int progressBarSpeed = 2; // Speed of the progress bar movement

void setup() {
  Serial.begin(9600);
  Serial.print("Key pressed: ");
  //Serial.println(key);
  tft.reset(); // Reset the display
  uint16_t ID = tft.readID(); // Read the display ID
  tft.begin(ID); // Initialize the display with the ID
  tft.fillScreen(0); // Clear the screen

  // Set text color and size
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);

  // Set display rotation to landscape
  tft.setRotation(3); // Use 1, 2, or 3 for landscape orientations
  bool good = SD.begin(SD_CS);
  if (!good){
    Serial.print(F("Cannot start SD"));
    while(1);
  }
  root = SD.open(namebuf);
  pathlen = strlen(namebuf);

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
          if (isPhoneNumberComplete){
            currentMenu = VERIFICATION_MENU;
            displayVerificationAnimation();
          }else{
            tft.println("Error! ");
          }
          // Always allow transition to the verification menu when '#' key is pressed
          /*currentMenu = VERIFICATION_MENU;
          displayVerificationAnimation();*/
        }
        break;
      case '*':
        if (currentMenu != HOME) {
          currentMenu = HOME;
          showBMP("Logo1.bmp", 0, 0);
          resetPhoneNumber(); // Reset the phone number when going back
        }
        break;
      case 'D':
        handleDeleteKeyPress();
        break;
      case '0' ... '9':
        // Handle numeric key presses
        handleNumericKeyPress(key);
        break;
      // Handle other keys and menu states here
    }
  }
  unsigned long currentMillis = millis();
    unsigned long loadingStartTime = millis();
    const unsigned long loadingDuration = 2000;
    // Calculate the elapsed time for the loading screen
    unsigned long loadingElapsedTime = currentMillis - loadingStartTime;

    if (loadingElapsedTime < loadingDuration) {
        // Display the loading screen until loadingDuration is reached
        //displayLoadingScreen();
    } else {
        if (bitmapStartTime == 0) {
            // Start tracking bitmap display time
            bitmapStartTime = currentMillis;
        }

        // Calculate the elapsed time for the bitmap display
        unsigned long bitmapElapsedTime = currentMillis - bitmapStartTime;

        if (bitmapElapsedTime < bitmapDuration) {
            // Display the bitmap for bitmapDuration
            showBMP("Logo1.bmp", 0, 0);
        } else {
            // After bitmapDuration, make the bitmap constant and allow access to the submenu
            // Other tasks can be added here
        }
    }

  // The rest of your loop function remains unchanged
}
#define BMPIMAGEOFFSET 54

#define BUFFPIXEL      20

uint16_t read16(File& f) {
    uint16_t result;         // read little-endian
    f.read(&result, sizeof(result));
    return result;
}

uint32_t read32(File& f) {
    uint32_t result;
    f.read(&result, sizeof(result));
    return result;
}


uint8_t showBMP(char *nm, int x, int y)
{
    File bmpFile;
    int bmpWidth, bmpHeight;    // W+H in pixels
    uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
    uint32_t bmpImageoffset;    // Start of image data in file
    uint32_t rowSize;           // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
    uint8_t bitmask, bitshift;
    boolean flip = true;        // BMP is stored bottom-to-top
    int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
    uint32_t pos;               // seek position
    boolean is565 = false;      //

    uint16_t bmpID;
    uint16_t n;                 // blocks read
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;               // off screen

    bmpFile = SD.open(nm);      // Parse BMP header
    bmpID = read16(bmpFile);    // BMP signature
    (void) read32(bmpFile);     // Read & ignore file size
    (void) read32(bmpFile);     // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);       // Start of image data
    (void) read32(bmpFile);     // Read & ignore DIB header size
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);        // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    pos = read32(bmpFile);      // format
    if (bmpID != 0x4D42) ret = 2; // bad ID
    else if (n != 1) ret = 3;   // too many planes
    else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette 
    else {
        bool first = true;
        is565 = (pos == 3);               // ?already in 16-bit format
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {              // If negative, image is in top-down order.
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())       // Crop area to be loaded
            w = tft.width() - x;
        if ((y + h) >= tft.height())      //
            h = tft.height() - y;

        if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
            //bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
            bmpFile.seek(bmpImageoffset - (4<<bmpDepth)); //54 for regular, diff for colorsimportant
            bitmask = 0xFF;
            if (bmpDepth < 8)
                bitmask >>= bmpDepth;
            bitshift = 8 - bmpDepth;
            n = 1 << bmpDepth;
            lcdbufsiz -= n;
            palette = lcdbuffer + lcdbufsiz;
            for (col = 0; col < n; col++) {
                pos = read32(bmpFile);    //map palette to 5-6-5
                palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
            }
        }

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) { // For each scanline...
            
            uint8_t r, g, b, *sdptr;
            int lcdidx, lcdleft;
            if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else        // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; ) {  //pixels in row
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
                    uint16_t color;
                    // Time to read more pixel data?
                    if (buffidx >= sizeof(sdbuffer)) { // Indeed
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0; // Set index to beginning
                        r = 0;
                    }
                    switch (bmpDepth) {          // Convert pixel from BMP to TFT format
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            color = tft.color565(r, g, b);
                            break;
                        case 16:
                            b = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (is565)
                                color = (r << 8) | (b);
                            else
                                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
                            break;
                        case 1:
                        case 4:
                        case 8:
                            if (r == 0)
                                b = sdbuffer[buffidx++], r = 8;
                            color = palette[(b >> bitshift) & bitmask];
                            r -= bmpDepth;
                            b <<= bmpDepth;
                            break;
                    }
                    lcdbuffer[lcdidx] = color;

                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }           // end cols
        }               // end rows
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
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
  tft.setCursor(40, 80);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.println("Pay Rent Menu");
  tft.setCursor(40, 180);
  tft.setTextColor(0xFF00);
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
  Serial.print("Key Pressed: ");
  Serial.println(key);
  if (key == '*') {
    currentMenu = HOME;
    displayHomePage();
    resetPhoneNumber(); // Reset the phone number when going back
  } else if (key == '#') {
    if (isPhoneNumberComplete) {
      // Handle submission of the phone number
      // You can add the code here to send/store the phone number
      // For now, let's simulate verification
      currentMenu = VERIFICATION_MENU;
      displayVerificationAnimation();
    }
    Serial.print("Key Pressed: ");
    Serial.println(key);
    
  } else if (key == 'D') {
    // Delete the last digit if the phone number is not empty
    if (phoneNumberIndex > 0) {
      phoneNumberIndex--;
      phoneNumber[phoneNumberIndex] = '\0'; // Null-terminate the string
      displayPhoneNumberInput(phoneNumber);
      // Update the phone number completion flag
      isPhoneNumberComplete = false;
    }
  } else if (phoneNumberIndex < 10 && isDigit(key)) {
    phoneNumber[phoneNumberIndex] = key;
    phoneNumberIndex++;
    phoneNumber[phoneNumberIndex] = '\0'; // Null-terminate the string
    displayPhoneNumberInput(phoneNumber);

    // Check if the phone number is complete
    if (phoneNumberIndex == 10) {
      isPhoneNumberComplete = true;
    }
  } else {
    // Handle other key presses in the phone number input menu (if needed)
  }
}
void resetPhoneNumber() {
  phoneNumberIndex = 0;
  phoneNumber[0] = '\0'; // Reset the phone number
}

void displayPhoneNumberInput(const char* phoneNumber) {
  tft.fillScreen(0); // Clear the screen
  tft.setCursor(20, 60);
  tft.setTextColor(0xFFFF); // White text color
  tft.setTextSize(2);
  tft.print("Enter Phone Number: ");
  tft.setCursor(20, 100);
  tft.print(phoneNumber);
  tft.setTextColor(0xFF00);
  tft.setCursor(40, 180);
  tft.print("Press # to submit or       * to go back");
}

void handleNumericKeyPress(char key) {
  if (currentMenu == PHONE_NUMBER_MENU && phoneNumberIndex < 10) {
    phoneNumber[phoneNumberIndex] = key;
    phoneNumberIndex++;
    phoneNumber[phoneNumberIndex] = '\0'; // Null-terminate the string
    displayPhoneNumberInput(phoneNumber);

    if (phoneNumberIndex ==10){
      isPhoneNumberComplete = true;
    }
  }
}

void handleDeleteKeyPress(){
  if (currentMenu == PHONE_NUMBER_MENU){
    if (phoneNumberIndex > 0){
      phoneNumberIndex--;
      phoneNumber[phoneNumberIndex] ='\0';
      displayPhoneNumberInput(phoneNumber);
      isPhoneNumberComplete = false;
    }
  }
}



int animationFrame = 0; // Variable to keep track of the animation frame
//unsigned long lastAnimationMillis = 0; // Variable to store the last animation update time

void displayVerificationAnimation() {
  // Clear the screen
  tft.fillScreen(0);
  
  // Calculate elapsed time since the last animation frame update
  unsigned long currentMillis = millis();
  if (currentMillis - lastAnimationMillis >= 500) {
    // Update the animation frame every 500 milliseconds (adjust the duration as needed)
    animationFrame = (animationFrame + 1) % 4;
    lastAnimationMillis = currentMillis;
  }

  tft.setTextColor(0x07E0); // Green text color (RGB565 format)
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.print("Verifying");

  loading(".", 3);

  // Implement a rotating line animation in green color
  //int centerX = tft.width() / 2;
  //int centerY = tft.height() / 2;
  //int lineLength = 20;
  //int lineX1, lineY1, lineX2, lineY2;

  // Calculate line coordinates based on the animation frame
 /* switch (animationFrame) {
    case 0:
      lineX1 = centerX - lineLength / 2;
      lineY1 = centerY;
      lineX2 = centerX + lineLength / 2;
      lineY2 = centerY;
      break;
    case 1:
      lineX1 = centerX;
      lineY1 = centerY - lineLength / 2;
      lineX2 = centerX;
      lineY2 = centerY + lineLength / 2;
      break;
    case 2:
      lineX1 = centerX - lineLength / 2;
      lineY1 = centerY - lineLength / 2;
      lineX2 = centerX + lineLength / 2;
      lineY2 = centerY + lineLength / 2;
      break;
    case 3:
      lineX1 = centerX - lineLength / 2;
      lineY1 = centerY + lineLength / 2;
      lineX2 = centerX + lineLength / 2;
      lineY2 = centerY - lineLength / 2;
      break;
  }*/

  // Draw the rotating line
  //tft.drawLine(lineX1, lineY1, lineX2, lineY2, 0x07E0);
}

/*void displayVerificationAnimation() {
  tft.fillScreen(0); // Clear the screen
  tft.setTextColor(0x07E0); // Green text color (RGB565 format)
  tft.setTextSize(2);
  tft.setCursor(60, 80);
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
}*/


/*void loading(char msg[]) {
  tft.setCursor(20, 80);
  tft.println(msg);

  for (int i = 0; i < 3; i++) {
    delay(1000);
    tft.print(".");
  }
}*/
void loading(const char*msg, int numDots){
  int textWidth = strlen("Verifying") * 12 * 2;
  int cursorX =20 + textWidth;
  int cursorY =60;
  tft.setCursor(cursorX, cursorY);

  for (int i =0; i<numDots; i++){
    tft.print(msg);
    delay(500);
    tft.setCursor(cursorX, cursorY);
    tft.print(msg);
    delay(500);
    //tft.setCursor(cursorX, cursorY);
  }
}