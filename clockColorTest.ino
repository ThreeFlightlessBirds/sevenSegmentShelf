//This Sketch was created to (relatively) easily/rapidly 
//test new colors for the digits without having to change
//the core Sketch for the project. The setup and some of 
//the functionality is directly borrowed from the main 
//Sketch, but uses static numbers that vary between two
//color sets every 5 seconds. I often use this to compare 
//8 colors at a time, setting the "bright" variation to
//4 different colors and doing the same for the "dim" 
//variation. Once a selected set of colors (I find different
//shades for the 10s and 1s place ensures the close placement 
//of these in my version of the clock is visually pleasing).
//Adafruit IO is not used, but it was convenient to use the
//same config.h file and keep the IO setup functions in the Sketch
//to facilitate WiFi connection for ArduinoOTA


#include "time.h"
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include <ArduinoOTA.h>

unsigned long previousMillis = 0;
bool showBright;

//pin for main clock
#define LEDCLOCK_PIN 13

//9*7*4+10, pixels/segment/digit + downlights
#define LEDCLOCK_COUNT 262
//total shelves

//color for "ones" place for left number
int leftOneColor;
//color for "tens" place for left number
int leftTenColor;
//color for "ones" place for right number
int rightOneColor;
//color for "tens" place for right number
int rightTenColor;
//color for the downlights
int downColor;

//max brightness
int clockFaceBrightness = 255;

//strip for the digits
Adafruit_NeoPixel stripClock(LEDCLOCK_COUNT, LEDCLOCK_PIN, NEO_GRB + NEO_KHZ800);

void digitZero(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 27);
    stripClock.fill(colour, (36 + offset), 27);
}

void digitOne(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 9);
    stripClock.fill(colour, (36 + offset), 9);
}

void digitTwo(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 18);
    stripClock.fill(colour, (27 + offset), 9);
    stripClock.fill(colour, (45 + offset), 18);
}

void digitThree(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 18);
    stripClock.fill(colour, (27 + offset), 27);
}

void digitFour(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 9);
    stripClock.fill(colour, (18 + offset), 27);
}

void digitFive(int offset, int colour){
    stripClock.fill(colour, (9 + offset), 45);
}

void digitSix(int offset, int colour){
    stripClock.fill(colour, (9 + offset), 54);
}

void digitSeven(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 18);
    stripClock.fill(colour, (36 + offset), 9);
}

void digitEight(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 63);
}

void digitNine(int offset, int colour){
    stripClock.fill(colour, (0 + offset), 45);
}


//returns a color value
int makeColor(int r, int g, int b){
  return r*65536 + g*256 +b;
}

//code from the project creator that sets up the pixels for each number
void displayNumber(int digitToDisplay, int offsetBy, int colorToUse){
  switch (digitToDisplay){
    case 0:
    digitZero(offsetBy,colorToUse);
      break;
    case 1:
      digitOne(offsetBy,colorToUse);
      break;
    case 2:
    digitTwo(offsetBy,colorToUse);
      break;
    case 3:
    digitThree(offsetBy,colorToUse);
      break;
    case 4:
    digitFour(offsetBy,colorToUse);
      break;
    case 5:
    digitFive(offsetBy,colorToUse);
      break;
    case 6:
    digitSix(offsetBy,colorToUse);
      break;
    case 7:
    digitSeven(offsetBy,colorToUse);
      break;
    case 8:
    digitEight(offsetBy,colorToUse);
      break;
    case 9:
    digitNine(offsetBy,colorToUse);
      break;
    default:
     break;
  }
}

void setup() {
  delay(3000);
  // connect to io.adafruit.com, and the rest of the internet apparently
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }

  //required for OTA updates, which seem to work sometimes...
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      //Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      //Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      //Serial.printf("Error[%u]: ", error);
      //if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      //else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      //else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      //else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      //else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  //start the neopixel strip and set the brightness, then light it up
  stripClock.begin();
  stripClock.setBrightness(clockFaceBrightness);

  //leftOneColor = makeColor(9, 1, 2);
  //leftTenColor = stripClock.gamma32(stripClock.ColorHSV(64443, 227, 10));
  //leftTenColor = makeColor(9, 2, 1);
  //rightOneColor = makeColor(9, 3, 1);
  //rightTenColor = stripClock.gamma32(stripClock.ColorHSV(62622, 227, 10);
  //rightTenColor = makeColor(5, 1, 0);
  //less bright white
  //downColor = stripClock.gamma32(stripClock.ColorHSV(0, 0, 127));
  
  //less bright white
  //downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 135)); 

  
  
  downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 135));
  
  stripClock.show();
}

void loop() {
  
  //check out OTA stuff
  ArduinoOTA.handle();

  //required for AdafruitIO
  //io.run();

  stripClock.clear();
  
  unsigned long currentMillis = millis();
  //change the 'brightness' setting every 5 seconds to make it easy
  //to test dim and bright, 2 different color sets, or even 8 colors;
  //as noted, I often test 8 colors at a time, then narrow the display 
  //to get a feel for a prospective palette
  if (currentMillis - previousMillis >= 5000){
    showBright = !showBright;
    previousMillis = currentMillis;
  }
  if (showBright){
    //doesn't HAVE to be the bright version of a palette, but often is
    leftTenColor = makeColor(45, 20, 4);
    leftOneColor = makeColor(50, 15, 3);
    rightTenColor = makeColor(55, 10, 2);
    rightOneColor = makeColor(60, 5, 1);
    //using 5 and 8, mostly because there's 1 overlapping line where 
    //the 10s and 1s place "touch" in my version of the clock
    displayNumber(5, 189, leftTenColor);
    displayNumber(8, 126, leftOneColor);
    displayNumber(5, 63, rightTenColor);
    displayNumber(8, 0, rightOneColor);
  } else { 
    //alternate color set or brightness variation
    leftTenColor = (stripClock.ColorHSV(4369, 232, 46)); 
    leftOneColor = (stripClock.ColorHSV(2912.71, 240, 51)); 
    rightTenColor = (stripClock.ColorHSV(1820, 245, 56)); 
    rightOneColor = (stripClock.ColorHSV(2549, 250, 61)); 
    
    displayNumber(6, 189, leftTenColor);
    displayNumber(9, 126, leftOneColor);
    displayNumber(6, 63, rightTenColor);
    displayNumber(9, 0, rightOneColor);
  }
  //can use to test variations on the downlighting color
  downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 135));
  stripClock.fill(downColor, 252, 10);
  stripClock.show();
}
