# clockShelf
Custom code for a customized build of DIY Machines Giant Hidden Shelf Edge Clock
[DIY Machines](https://www.diymachines.co.uk/how-to-build-a-giant-hidden-shelf-edge-clock)

While the original build is a phenomenal work of art, I wanted to add some additional features not included in the original project. Specifically, I wanted a non-physical way to control the lighting. 

## Design Decisions
As WiFi was the logical choice for a project that would live in a living room (or eventually a bedroom, making the ability to turn the lights off even more crucial), this meant the Arduino Nano from the original project wouldn't cut it. With previous experience using (often very bare-bones) ESP8266 breakout and development boards in the past, and the readily available plethora of ESP32 boards becoming available when I began this project in 2019, the SparkFun [SparkFun Thing Plus - ESP32 WROOM](https://www.sparkfun.com/products/17381) was put into service (mostly because I really respect and like [SparkFun](https://www.sparkfun.com/about_sparkfun)).

To facilitate indoor temperature and humitidy data, a generic DHT22 breakout sensor was originally used because there was a handful available in my parts bin. Eventually, I updated the project to work with the [SparkFun - SHTC3 (Qwiic)](https://www.sparkfun.com/products/16467) because it was trivial to implement in the project, had a Qwiic connecter (available on the Thing Plus), and was more reliable + accurate than the DHT22 sensor. 

I settled on [Adafruit IO](https://io.adafruit.com/) because it is free (with reasonable limitations), available for Arduino and Micro/Circuit Python (I've always considered porting the project to run on a Pico W or similar), and quite easy to get started with. Once this decision had been made, I decided to include temperature and humidity data in the project, both internally via a sensor and externally via OpenWeatherMap. 

Using Adafruit IO accomplishes connecting to the internet, and at the time this project was created [OpenWeatherMap](https://openweathermap.org/api) was the service recommended by Adafruit for use with their cloud offering (though they have since changed to using Apple WeatherKit). 

