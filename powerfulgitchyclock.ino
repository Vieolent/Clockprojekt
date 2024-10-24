/*
* Name: clock and temp project
* Author: Miranda Bache
* Date: 2024-10-24
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display.
* There is a button that displays the time as an hour earlier as it actually is, so that you are never late.
* Further, it measures temprature with a analog temprature module and displays a mapped value to a 9g-servo-motor
* It is also able to display both seconds in a minute and the temperature, mapped onto a neopixel ring.
*/

// Include Libraries
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <U8glib.h>
#include <RTClib.h>
#include <Servo.h>

// Init constants
const int tempPin = A0;

// Init global variables
float angle = 0;
int hour = 0;
int minute = 0;

// construct objects
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
Servo myservo;
RTC_DS3231 rtc;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, 8, NEO_GRB + NEO_KHZ800);

void setup() {
  // init communication
  Wire.begin();

  // Init Hardware
  myservo.attach(6);
  pinMode(2, INPUT);
  pinMode(10, INPUT);
  pinMode(A0, INPUT);
  pixels.begin();
  rtc.begin();

  u8g.setFont(u8g_font_unifont);
  pixels.setBrightness(50);

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {

  oledWrite(getTime(), "The current time:", "100% accurate");
  servoWrite(map(getTemp(), 10, 34, 0, 179));

  if (digitalRead(10) == HIGH) {
    ringWriteTemp(getTemp());
  } else {
    ringWriteTime();
  }
  delay(1000);
}

/*
*This function reads time from an ds3231 module and package the time as a String
*Parameters: Void
*Returns: time in hh:mm:ss as String
*/
String getTime() {
  DateTime now = rtc.now();
  minute = now.minute();
  hour = now.hour();
  if (digitalRead(2) == HIGH) {
    hour = now.hour() - 1;
  }
  return (String(hour) + ":" + String(minute) + ":" + String(now.second()));
}

/*
* This function reads an analog pin connected to an analog temprature sensor and calculates the corresponding temp
*Parameters: Void
*Returns: temprature as float
*/
float getTemp() {
  float temp = 0;
  float R1 = 10000;  // value of R1 on board
  float logR2, R2;
  float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  //steinhart-hart coeficients for thermistor

  int Vo = analogRead(tempPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);  //calculate resistance on thermistor
  logR2 = log(R2);
  temp = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));  // temperature in Kelvin
  temp = temp - 273.15;
  //convert Kelvin to Celcius

  return temp;
}

/*
* This function takes a string and draws it to an oled display
*Parameters: - text: String to write to display
*Returns: void
*/
void oledWrite(String text, String text1, String text2) {
  u8g.firstPage();
  do {
    u8g.drawStr(32, 37, text.c_str());
    u8g.drawStr(0, 10, text1.c_str());
    u8g.drawStr(10, 62, text2.c_str());
  } while (u8g.nextPage());
}

/*
* takes a temprature value and maps it to corresppnding degree on a servo
*Parameters: - value: temperature
*Returns: void
*/
void servoWrite(float value) {
  myservo.write(value);
}
/*
* displays seconds as a light moving around a circle. A full lap takes 1 minute. 
*Parameters: - void
*Returns: void
*/
void ringWriteTime() {
  ringWipe();
  for (int i = 0; i <= map(rtc.now().second(), 0, 59, 0, 23); i++) {
    pixels.setPixelColor(i, pixels.Color(0, 80, 0));
    if (i != 0) {
      pixels.setPixelColor(i - 1, pixels.Color(40, 40, 40));
    }
    pixels.show();
  }
}
/*
*Displays the temperature as an ammount of lights in a circle. One light is one degree celsius + 10 degrees to start. 
*This only works for temperatures between 10 and 34 degrees celcius. 
*Parameters: - value: temperature
*Returns: - void
*/

void ringWriteTemp(int num) {
  int t = map(num, 10, 34, 0, 23);
  int shadeNow = map(int(minute), 0, 59, 20, 255);

  ringWipe();
  for (int i = 0; i <= t; i++) {
    pixels.setPixelColor(i, pixels.Color(shadeNow, 0, 0));
    pixels.show();
    Serial.println(shadeNow);
  }
}
/*
*Resets the light ring to display all lights as white.
*Parameters: - void
*Returns: - void
*/
void ringWipe() {
  for (int i = 0; i <= 23; i++) {
    pixels.setPixelColor(i, pixels.Color(40, 40, 40));
    pixels.show();
  }
}
