# clockShelf
Custom code for a customized build of DIY Machines Giant Hidden Shelf Edge Clock
[DIY Machines](https://www.diymachines.co.uk/how-to-build-a-giant-hidden-shelf-edge-clock)

While the original build is a phenomenal work of art, I wanted to add some additional features not included in the original project. Specifically, I wanted a non-physical way to control the lighting. 

## Design Decisions
As WiFi was the logical choice for a project that would live in a living room (or eventually a bedroom, making the ability to turn the lights off even more crucial), this meant the Arduino Nano from the original project wouldn't be sufficient. With previous experience using (often very bare-bones) ESP8266 breakout and development boards in the past, and the readily available plethora of ESP32 boards becoming available when I began this project in 2019, the SparkFun [SparkFun Thing Plus - ESP32 WROOM](https://www.sparkfun.com/products/17381) was put into service (mostly because I really respect and like [SparkFun](https://www.sparkfun.com/about_sparkfun)).

I settled on [Adafruit IO](https://io.adafruit.com/) because it is free (with reasonable limitations), available for Arduino and Micro/Circuit Python (I've always considered porting the project to run on a Pico W or similar), and quite easy to get started with. Once this decision had been made, I decided to include temperature and humidity data in the project, both internally via a sensor and externally via OpenWeatherMap. 

Using Adafruit IO accomplishes connecting to the internet, and at the time this project was created [OpenWeatherMap](https://openweathermap.org/api) was the service recommended by Adafruit for use with their cloud offering (though they now offer services using Apple WeatherKit). OpenWeatherMap is similarly easy to get started with, and the incredibly generous limitations make the free services more than capable for such a project. The data returned from the API is in easy-to-use JSON format. 

To facilitate indoor temperature and humitidy data, a generic DHT22 breakout sensor was originally used because there was a handful available in my parts bin. Eventually, I updated the project to work with the [SparkFun - SHTC3 (Qwiic)](https://www.sparkfun.com/products/16467) because it was trivial to implement in Arduino, had a Qwiic connecter (available on the Thing Plus), and was more reliable + accurate than the DHT22 sensor. 

## Setup
To use this project, some placeholder values need to be updated with real values. Starting in the config.h file, update your WiFi credentials (WIFI_SSID and WIFI_PASS). You'll also need your Adafruit IO Username and Adafruit IO Key (Available in your Profile once you've set up an account). At the time of this writing, [the official guide](https://learn.adafruit.com/welcome-to-adafruit-io) is returning a 404 message, but the "[outdated](https://learn.adafruit.com/adafruit-io/getting-started)" version should work well enough, and the example projects for Adafruit IO also contain helpful information.

Back in the main Sketch, you'll need to update the weatherPath string with your zip code and API key. There are many ways of calling the API, and you may notice that the Sketch uses version 2.5 of the API while at the time of this writing version 3.0 is available. Feel free to check out the [OpenWeatherMap documentation](https://openweathermap.org/api/one-call-3#current) and make adjustments if needed. 

Otherwise, everything else should "just work" if the physical setup is the same. Pin 13 is used for the NeoPixel data line, and there are 262 total LEDs in my version (9 LEDs per segment, 7 segments per digit, 4 digits, plus 10 downlights). The SHTC3 is connected via the Qwiic (I2C) connectors on the breakout and Thing Plus boards. 

## Build details
[forthcoming]
