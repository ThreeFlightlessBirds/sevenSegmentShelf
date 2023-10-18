//This code is written for the Sparkfun Thing Plus, https://www.sparkfun.com/products/15663; 
//but could probaly work with any number of ESP32 dev boards 
#include <HTTPClient.h>
#include "time.h"
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include <ArduinoOTA.h>
#include <Arduino_JSON.h>
#include <Wire.h>//needed for sensors
#include "SparkFun_SHTC3.h" 

//using a static path for weather data, as it shouldn't need to change
//http://api.openweathermap.org/data/2.5/weather?zip=**YOUR_ZIP_CODE**,us&&units=imperial&appid=**YOUR_APP_ID";
static String weatherPath = "[YOUR OPEN WEATHERMAP PATH]";
//variable for outside temp
double outTemp;
//used to check if the outside temp has changed, as the feed isn't updated if there's no change to reduce API calls
double prevOutTemp;
//variable for outside humidity
double outHum;

//value used to ensure IO feeds only updated once every 10 seconds 
bool webUpdate = true;

//temp&humidity sensor; I'm using this: https://www.sparkfun.com/products/16467
SHTC3 mySHTC3;
//variable to hold temp reading from sensor
float intTemp;
//variable to hold humidity reading from sensor
float intHum;

//delay() replacement to help with wifi, OTA, AfIO tasks
unsigned long previousMillis = 0;

//pin for main clock
#define LEDCLOCK_PIN 13

//9*7*4+10, pixels*segments*digit + downlights
#define LEDCLOCK_COUNT 262

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

//max brightness, can lower by adjusting color and turning off individual pixels (to be implemented); should only be assigned once during Setup
int clockFaceBrightness = 255;

//strip for the digits
Adafruit_NeoPixel stripClock(LEDCLOCK_COUNT, LEDCLOCK_PIN, NEO_GRB + NEO_KHZ800);

//set up the AdafruitIO feeds
AdafruitIO_Feed *onoff = io.feed("on-slash-off");
AdafruitIO_Feed *temperature = io.feed("lrtemp");
AdafruitIO_Feed *humidity = io.feed("lrhum");
AdafruitIO_Feed *osTemp = io.feed("ostemp");
AdafruitIO_Feed *osHum = io.feed("oshum");
AdafruitIO_Feed *ioBrightness = io.feed("brightness");
AdafruitIO_Feed *temphum = io.feed("temphumdown");
AdafruitIO_Feed *downLights = io.feed("downlights");

//whether any lights should be on
bool aioOnOff;
//whether the Brightness button is set to High or Low in Adafruit IO; currently supports 2 settings
bool aioBrightness; 
//whether the Temp/Humidity are set to be displayed
bool aioTemphum;
//whether the downlights are on
bool aioDownLights;

//NTP server address to get the time for RTC
const char* ntpServer = "pool.ntp.org";
//offset for mountain time
const long gmtOffset_sec = -25200;
//we do daylight savings
const int daylightOffset_sec = 3600;

//seconds
int secs;
//minutes
int theMin;
//ones place for minutes
int minOne;
//tens place for minutes
int minTen;
//holds the hour
int hr;
//hour ones place
int hrOne;
//hour tens place
int hrTen;

void setup() {
  Serial.begin(115200);
  //delay for recovery, since everyone does it
  delay(3000);
  
  // connect to io.adafruit.com, and the rest of the internet even
  io.connect();

  // wait for a connection to Adafruit IO
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
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

  //for the sensors
  Wire.begin();

  //start the temp&humidity sensor
  mySHTC3.begin();
  //set the mode to Clock stretching, Temp first, Normal power mode
  mySHTC3.setMode(SHTC3_CMD_CSE_TF_NPM);
  //set the sensor to staty awake
  mySHTC3.wake(true);

  //set the handlers for incoming messages from Adarfruit IO
  temphum->onMessage(handleTemphum);
  downLights->onMessage(handleDownLights);
  ioBrightness->onMessage(handleIOBrightness);
  onoff->onMessage(handleOnOff);
  //set the getters for Adafruit IO
  temphum->get();
  downLights->get();
  ioBrightness->get();
  onoff->get();

  //set up the time from the NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  //do the time thing for the first time, setting the digits to display
  setDisplayAndLog();
  //start the neopixel strip and set the brightness, then light it up
  stripClock.begin();
  stripClock.setBrightness(clockFaceBrightness);
  stripClock.show();
  
  //delay to the next "ten" seconds to try to sync minute change better
  delay((10 - (secs%10))*1000);
}

void loop() {
  
  //check out OTA stuff
  ArduinoOTA.handle();

  //required for AdafruitIO
  io.run();

  //temporary variable to check the current run time
  unsigned long currentMillis = millis();
  //compare the current run time to the previous run time to see if the magic 1000 milliseconds has passed
  if (currentMillis - previousMillis >= 1000) {
    //record the last time the time was updated 
    previousMillis = currentMillis;
    //do the time thing first
    setDisplayAndLog();
    //show the world!
    stripClock.show();
  }
}

//returns a color value
int makeColor(int r, int g, int b){
  return r*65536 + g*256 +b;
}

//gets new sensor data, sends updates to Adafruit IO
void ioAndOpenWeather() {
  //update the temp&humidity sensor and set the variables
      mySHTC3.update();
      intTemp = mySHTC3.toDegF();
      intHum = mySHTC3.toPercent();
      //update IO feeds
      temperature->save(intTemp);
      humidity->save(intHum);
      //OpenWeatherMap data grab; since the data only updates periodically, updates are only sent to Adafruit IO when new data is available to reduce API calls
      httpGETosTempHum();
      if (outTemp != prevOutTemp) {
        osTemp->save(outTemp);
        osHum->save(outHum);
        prevOutTemp = outTemp;
      }
}

//used to turn lights on or off when updated data available from Adafruit IO
void handleOnOff(AdafruitIO_Data *data){
  if (data->toString() == "High"){
    aioOnOff = true;
  } else {
    aioOnOff = false;
  }
}

//used to determine whether the temperature and humidity should display
void handleTemphum(AdafruitIO_Data *data){
  if (data->toString() == "ON"){
    aioTemphum = true;
  } else {
    aioTemphum = false;
  }
}

//used to determine whether the downlights should be on
void handleDownLights(AdafruitIO_Data *data){
  if (data->toString() == "ON"){
    aioDownLights = true;
  } else {
    aioDownLights = false;
  }
}

//used to determine whether the brightness should be High or Low
void handleIOBrightness(AdafruitIO_Data *data){
  if (data->toString() == "High"){
    aioBrightness = true;
  } else {
    aioBrightness = false;
  }
}

//where the magic happens!
//gets the time from the RTC
void setDisplayAndLog(){
  //structure from time.h
  struct tm timeinfo;
  //if the time can't get got, don't try the rest of the stuff
  if(!getLocalTime(&timeinfo)){
    return;
  }

  //get the seconds
  secs = timeinfo.tm_sec;
  //get the hour
  hr = timeinfo.tm_hour;

  //change the hour to 12 hour format, if needed
  if (hr > 12) { hr = hr-12;}
  //show "12" at midnight instead of "0"
  if (hr == 0) {hr = 12;}
  //break the hour into to separate digits
  hrOne = hr % 10;
  hrTen = hr/10;

  //break the minutes into separate digits
  minOne = timeinfo.tm_min % 10;
  minTen = timeinfo.tm_min/10;
  
  //for the first 9 seconds surrounding the start of each minute...
  if (secs < 5 || secs >55){
    //do IO stuff if we haven't yet, get the OpenWeatherMap data
    //and update the sensors once 
    if (webUpdate){
      //update Adafruit IO and get Open Weather Map data
      ioAndOpenWeather();
      //then hold off until the next minute
      webUpdate = false;
    }

    //check if any lights should be on
    if (aioOnOff) {
      //check the option to display temp/hum data
      if (aioTemphum) {
        //display based on the brightness setting
        if (aioBrightness){
          brightTemp();
        } else{
            dimTemp();
        }
        
      } else {//show the time, if temp and humidity not enabled, and still check the brightness
        //display based on the brightness setting
        if (aioBrightness){
          brightTime();
        } else{
          dimTime();
        }
      }
      
      //check the option to display downlights; not needed as the downlights are controlled by their "color" now
      //if (aioDownLights){
        
      //} else {
        
      //}
      
    } else {
      offTime();
    }
   }
    //for 8 seconds surrounding the half-minute, show humidity info 
    else if (secs > 25 && secs < 35) {
      if (aioOnOff){
        //check the option display temp/hmidity and turn the downlights on
        if (aioTemphum) {
          //display based on the brightness setting
          if (aioBrightness){
            brightHum();
          } else{
              dimHum();
          }
            
        } else {//otherwise display the time, and still check the brightness
            if (aioBrightness) {
              brightTime();
            } else{
              dimTime();
            }
        }
      } else {
        offTime();
      }
      
      
    } 
  //otherwise show the time
  else {
    
    //once we're away from the code to send feed data, 
    //we can reset the check and display the time instead 
    webUpdate = true;

    
  
    //(red * 65536) + (green * 256) + blue ->for 32-bit merged colour value so 16777215 equals white
  
    if (aioOnOff){
      //display based on the brightness setting
      if (aioBrightness) { 
        brightTime();
      } else { 
        dimTime();
      }
    } else { //if lights are off, sets lights to off
      offTime();
    }
    
    
  }
  //clear the clock, and then set the pixels to display
  stripClock.clear();
  //not displaying a "0" in the front of the hour!
  if (hrTen>0) {displayNumber(hrTen, 189, leftTenColor);}
  displayNumber(hrOne, 126, leftOneColor);
  displayNumber(minTen, 63, rightTenColor);
  displayNumber(minOne, 0, rightOneColor);
  
  //set the downlights
  stripClock.fill(downColor, 252, 10);

}

//code from the project creator that sets up the pixels for each number; see digits.ino for more info
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

//code to get Open Weather Map data
void httpGETosTempHum() {
  HTTPClient http;
  
  // Your IP address with path or Domain name with URL path 
  http.begin(weatherPath.c_str());
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = http.getString();

  // Free resources
  http.end();

  JSONVar myObject = JSON.parse(payload);
  
  outTemp = myObject["main"]["temp"];
  outHum = myObject["main"]["humidity"];
}

//if the clock face lights are off, set the color to "black"
void offTime(){
  leftTenColor = makeColor(0, 0, 0);
  leftOneColor = makeColor(0, 0, 0);
  rightTenColor = makeColor(0, 0, 0);
  rightOneColor = makeColor(0, 0, 0);
  downColor = makeColor(0, 0, 0);
}

//display the temperature, dimly; also sets the digits
void dimTemp(){
  //round up the temps for integer conversion
  hr = (int) (outTemp + .05);
  theMin = (int) (intTemp + 0.5);
  //break the numbers into digits
  hrOne = hr % 10;
  hrTen = hr/10;
  minOne = theMin % 10;
  minTen = theMin / 10;

  leftTenColor = makeColor(7, 1, 0);
  leftOneColor = makeColor(8, 2, 0);
  rightTenColor = makeColor(10, 1, 0);
  rightOneColor = makeColor(6, 0, 0);
  //check if down lights should be on
  if (aioDownLights) {
    //less bright yellow
    downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 135));
    //downColor = makeColor(110, 70, 40);
  } else {
    downColor = makeColor(0, 0, 0);
  }
}

//display the temp brightly; also sets the digits
void brightTemp(){
  leftTenColor = makeColor(57, 3, 0);
  leftOneColor = makeColor(56, 14, 0);
  rightTenColor = makeColor(50, 5, 0);
  rightOneColor = makeColor(55, 0, 0);
  if (aioDownLights) {
    //trying yellow for the downlights
    downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 225));
    //downColor = makeColor(220, 80, 20);
  } else {
    downColor = makeColor(0, 0, 0);
  }

  //round up the temps for integer conversion
  hr = (int) (outTemp + .05);
  theMin = (int) (intTemp + 0.5);
  //break the numbers into digits
  hrOne = hr % 10;
  hrTen = hr/10;
  minOne = theMin % 10;
  minTen = theMin / 10;
}

//displays the humidity dimly; also sets the digits
void dimHum(){
  leftTenColor = makeColor(1, 3, 2);
  leftOneColor = makeColor(0, 2, 3);
  rightTenColor = makeColor(1, 2, 3);
  rightOneColor = makeColor(0, 1, 2);
  if (aioDownLights) {
    //less bright yellow
    downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 135)); 
    //downColor = makeColor(110, 70, 40);
  } else {
    downColor = makeColor(0, 0, 0);
  }

  //round up the humidities for integer conversion
  hr = (int) (outHum + .05);
  theMin = (int) (intHum + 0.5);
  //if the outdoor humidity is 100%, display it as 99
  if (hr == 100){
    hr = 99;
  }
  
  //break the numbers into digits
  hrOne = hr % 10;
  hrTen = hr/10;
  minOne = theMin % 10;
  minTen = theMin / 10;
}

//displays the humidity brightly; also sets the digits
void brightHum(){
  leftTenColor = makeColor(10, 30, 30);
  leftOneColor = makeColor(0, 20, 30);
  rightTenColor = makeColor(10, 20, 30);
  rightOneColor = makeColor(0, 10, 20);
  if (aioDownLights) {
    //trying yellow for the downlights
    downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 225));
    //downColor = makeColor(220, 80, 20);
  } else {
    downColor = makeColor(0, 0, 0);
  }

  //round up the humidities for integer conversion
  hr = (int) (outHum + .05);
  theMin = (int) (intHum + 0.5);

  //if the outdoor humidity is 100%, display it as 99
  if (hr == 100){
    hr = 99;
  }
  //break the numbers into digits
  hrOne = hr % 10;
  hrTen = hr/10;
  minOne = theMin % 10;
  minTen = theMin / 10;
}

//dim colors for the time
void dimTime(){
  if (aioTemphum){
    if (aioDownLights) {
      downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 135));
      //downColor = makeColor(110, 70, 40);
    } else {
      downColor = makeColor(0, 0, 0);
    }
    leftTenColor = makeColor(9, 4, 1);
    leftOneColor = makeColor(15, 5, 1);
    rightTenColor = makeColor(18, 3, 1);
    rightOneColor = makeColor(15, 1, 0);
  } else {
    downColor = makeColor(0, 0, 0);
    leftTenColor = makeColor(3, 1, 0);
    leftOneColor = makeColor(1, 0, 0);
    rightTenColor = makeColor(3, 1, 0);
    rightOneColor = makeColor(1, 0, 0);
  }
}

//bright colors for the time
void brightTime(){
  leftTenColor = makeColor(45, 20, 4);
  leftOneColor = makeColor(50, 15, 3);
  rightTenColor = makeColor(55, 10, 2);
  rightOneColor = makeColor(60, 5, 1);

  if (aioDownLights) {
    //trying yellow for the downlights
    downColor = stripClock.gamma32(stripClock.ColorHSV(10922, 70, 225));
    //downColor = makeColor(220, 80, 20);
  } else {
    downColor = makeColor(0, 0, 0);
  }
}
