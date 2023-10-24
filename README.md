# clockShelf
Custom code for a customized build of DIY Machines Giant Hidden Shelf Edge Clock
[DIY Machines](https://www.diymachines.co.uk/how-to-build-a-giant-hidden-shelf-edge-clock)
![IMG_9186](https://github.com/ThreeFlightlessBirds/clockShelf/assets/66700475/983e8376-005c-4564-8da1-01b66969612b)


While the original build is a phenomenal work of art, I wanted to add some additional features not included in the original project. Primarily, I wanted a non-physical way to control the lighting, but displaying temperature and humidity data was also desirable.

## Design Decisions
I made some fairly minor changes to the design to ease the overall material selection/acquisition process and simplify the assembly. When selecting the vertical placement of the shelves, I adjusted lower-than-center to accomodate items that didn't fit in the shelves to sit on top of the shelves. For that initial purpose, this visually balanced the final display, but makes it look odd when bare.

To fit the size of project board I could source in-stock locally, I decided to combine two segments to go where two digits meet, eliminating the need to have additional space between the numbers. While this does not photograph well, the small space between the adjacent segments is clearly visible in-person. Using slightly different hues for the adjacent digits also improved the distinction when two adjacent segments are lit. 

Testing various color palettes necessitated a way to (relatively) quickly update the lighting with a new Sketch. [clockColorTest.ino](https://github.com/ThreeFlightlessBirds/clockShelf/blob/main/clockColorTest.ino) is used for this, and needs to be in it's own Arduino project along with config.h to function. The digits.ino file does not currently need to be included in the project, as the code is included in the sketch, but this will likely chagce as I add additional dimming options/methods to the project.

To streamline the printing process and avoid non-trivial color changes printing the project on an Ender 3 V2, the faces of the sleeves were printed separately and glued to the faceless sleeves during assembly. I will likely remix the Thingiverse project to make these files available, and will update the README once completed.

As WiFi was the logical choice for a project that would live in a living room (or occaisionally a bedroom, making the ability to turn the lights off even more crucial), this meant the Arduino Nano from the original project wouldn't be sufficient. With previous experience using (often very bare-bones) ESP8266 breakout and development boards in the past, and the readily available plethora of ESP32 boards becoming available when I began this project in 2019, the SparkFun [SparkFun Thing Plus - ESP32 WROOM](https://www.sparkfun.com/products/17381) was put into service (mostly because I really respect and like [SparkFun](https://www.sparkfun.com/about_sparkfun)).

I settled on [Adafruit IO](https://io.adafruit.com/) because it is free (with reasonable limitations), available for Arduino and Micro/Circuit Python (I've always considered porting the project to run on a Pico W or similar), and quite easy to get started with. 

I elected to remove the face lighting on the shelves that were not part of the 4 "digits." This visually separated the display into two distinct two-digit sections. The lefthand pair are used to display the outdoor data, and the righthand pair display the indoor data. Thought was given to displaying indoor and outdoor data together, but it would be non-trivial to distinguish between them. Instead, for a few seconds just before and after the start of a minute, the temperature data is displayed. For a few seconds just before and after the 30 second mark, the humidity data is displayed. The left two digits display the outdoor temperature and humidity from OpenWeatherMap, while the right two digits display the indoor data fromt he sensor. 

Using Adafruit IO accomplishes connecting the ESP32 to the internet, and at the time this project was created [OpenWeatherMap](https://openweathermap.org/api) was the service recommended by Adafruit for use with their cloud offering (though they now offer services using Apple WeatherKit). OpenWeatherMap is similarly easy to get started with, and the incredibly generous limitations make the free services more than capable for such a project. The data returned from the API is in easy-to-use JSON format. 

To facilitate indoor temperature and humitidy data, a generic DHT22 breakout sensor was originally used because there was a handful available in my parts bin. Eventually, I updated the project to work with the [SparkFun - SHTC3 (Qwiic)](https://www.sparkfun.com/products/16467) because it was trivial to implement in Arduino, had a Qwiic connecter (available on the Thing Plus), and was more reliable + accurate than the DHT22 sensor. With either sensor, accuracy was greatly increased having the sensor outside of the shelves. I simply "dangle" the sensor through a slot in the upper rightmost vertical segment, but this could certainy be more elegantly accomplished (e.g. modifying the unlit top horizontal piece to have an open space to slot the sensor, though the additional length of the I2C/Qwiic cable could have an effect). 

## Setup
To use this project, some placeholder values need to be updated with real values. Starting in the config.h file, update your WiFi credentials (WIFI_SSID and WIFI_PASS). You'll also need your Adafruit IO Username and Adafruit IO Key (Available in your Profile once you've set up an account). At the time of this writing, [the official guide](https://learn.adafruit.com/welcome-to-adafruit-io) is returning a 404 message, but the "[outdated](https://learn.adafruit.com/adafruit-io/getting-started)" version should work well enough, and the example projects for Adafruit IO also contain helpful information.

Back in the main Sketch, you'll need to update the weatherPath string with your zip code and API key. There are many ways of calling the API, and you may notice that the Sketch uses version 2.5 of the API while at the time of this writing version 3.0 is available. Feel free to check out the [OpenWeatherMap documentation](https://openweathermap.org/api/one-call-3#current) and make adjustments if needed. 

Otherwise, everything else should "just work" if the physical setup is the same. Pin 13 is used for the NeoPixel data line, and there are 262 total LEDs in my version (9 LEDs per segment, 7 segments per digit, 4 digits, plus 10 downlights). The SHTC3 is connected via the Qwiic (I2C) connectors on the breakout and Thing Plus boards. 

## Build details
[forthcoming]
