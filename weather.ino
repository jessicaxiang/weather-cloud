#define DEBUG    1

#include <Adafruit_NeoPixel.h>
#include <AltSoftSerial.h>
#include <avr/power.h>
#include "States.h"

// Pin definitions
#define CLOUD_A_PIN       6
#define CLOUD_B_PIN       7

#define bitpin_1 10
#define bitpin_2 11
#define bitpin_3 12
#define signalpin 5

// Other constants
const int TWINKLE_DELAY = 400;    // ms
const int IN_BUF_MAX = 7;         // bytes
const byte SERIAL_SOF = 0xA5;
const byte SERIAL_EOF = 0x5A;
const int TIMEOUT = 100;          // ms
const int DISCO_DELAY = 85;       // ms
const byte COMM_RGB = 1;
const byte COMM_DISCO = 2;
const byte COMM_BLUE_SKY = 3;
const byte COMM_NIGHT = 4;
const byte COMM_OVERCAST = 5;
const byte COMM_GOLDEN = 6;
const byte COMM_SNOW = 7;
const byte COMM_LIGHTNING = 8;
const byte COMM_RAINBOW = 9;
const byte COMM_OFF = 10;
int bit1;
int bit2;
int bit3;
int signalbit;
int weather;

// Global variables
//SoftwareSerial softy(4, 5); // RX, TX
AltSoftSerial softy;  // RX = 8, TX = 9, do not use pin 10
unsigned long led_time;

Adafruit_NeoPixel strip_a = Adafruit_NeoPixel(30, CLOUD_A_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_b = Adafruit_NeoPixel(30, CLOUD_B_PIN, NEO_GRB + NEO_KHZ800);

/*
Adafruit_NeoPixel strip_b = Adafruit_NeoPixel(300, CLOUD_A_PIN, 
                                        NEO_GRB + NEO_KHZ800);
                                        
Adafruit_NeoPixel strip_a = Adafruit_NeoPixel(300, CLOUD_B_PIN, 
                                        NEO_GRB + NEO_KHZ800);*/
LEDState led_state;
NightState night_state;
int night_i;
int storm_state;
int storm_delay;
int disco_state;
int disco_delay;
byte in_msg[IN_BUF_MAX];
uint8_t bytes_received;
uint8_t color_r;
uint8_t color_g;
uint8_t color_b;

/****************************************************************
 * Setup
 ***************************************************************/
void setup() {
  
  // Set up debug serial console
  Serial.begin(9600);

  // Set up software serial
  softy.begin(9600);
  
  // Configure LED pins
  pinMode(CLOUD_A_PIN, OUTPUT);
  pinMode(CLOUD_B_PIN, OUTPUT);
  pinMode(signalpin, INPUT);
  pinMode(bitpin_1, INPUT);
  pinMode(bitpin_2, INPUT);
  pinMode(bitpin_3, INPUT);

  digitalWrite(signalpin, HIGH);
  digitalWrite(bitpin_1, HIGH);
  digitalWrite(bitpin_2, HIGH);
  digitalWrite(bitpin_3, HIGH);
  
  // Define initial state
  led_time = millis();
  led_state = LED_DISCO;
  storm_state = 0;
  disco_state = 0;
  disco_delay = DISCO_DELAY;
  night_state = NIGHT_0;
  night_i = 1;
  color_r = 0;
  color_g = 0;
  color_b = 0;
  
  // Set initial random delay
  pinMode(A0, INPUT);
  randomSeed(analogRead(A0));
  storm_delay = random(900, 1500);
  
  // Clear LED strips
  strip_a.begin(); 
  strip_a.show();
  strip_b.begin();
  strip_b.show();
  clearCloud();
}

/****************************************************************
 * Loop
 ***************************************************************/
void loop() {
  doLED();
  signalbit = digitalRead(signalpin);
  bit1 = digitalRead(bitpin_1);
  bit2 = digitalRead(bitpin_2);
  bit3 = digitalRead(bitpin_3);
  if(signalbit == 1){
    if(bit1 == 1){
      if(bit2 == 1){
        if(bit3 ==1){
          weather = 7;
        }
        else if(bit3 == 0){
          weather = 6;
        }
      }
      else if (bit2 == 0){
        if (bit3 == 1){
          weather = 5;
        }
        else if (bit3 == 0){
          weather = 4;
        }
      }
    }
    else if(bit1 == 0){
      if(bit2 == 1){
        if(bit3 == 1){
          weather = 3;
        }
        else if(bit3 == 0){
          weather = 2;
        }
      }
      else if (bit2 == 0){
        if (bit3 == 0){
          weather = 8;
        }
        else if (bit3 == 1){
          weather = 1;
        }
      }
    }
  }
    switch ( weather ) {
      case 1:
        led_state = LED_BLUE_SKY;
        break;
      case 2:
        night_state = NIGHT_0;
        led_state = LED_NIGHT;
        break;
      case 3:
        led_state = LED_OVERCAST;
        break;
      case 4:
        led_state = LED_GOLDEN;
        break;
      case 5:
        led_state = LED_SNOW;
        break;
      case 6:
        clearCloud();
        led_state = LED_LIGHTNING;
        break;
      case 7:
        led_state = LED_RAINBOW;
        break;
      case 8:
        led_state = LED_OFF;
        break;
      default:
        break;
    }
    Serial.println(weather);
  }

/****************************************************************
 * LED Functions
 ***************************************************************/

// Update LED strips
void doLED() {
  
  // Display LEDs based on the current weather
  switch ( led_state ) {

    // RGB
    case LED_RGB:
      break;

    // Disco
    case LED_DISCO:
      disco();
      break;
    
    // Blue sky
    case LED_BLUE_SKY:
      blueSky();
      break;
      
    // Night
    case LED_NIGHT:
      nighttime();
      break;
      
    // Overcast
    case LED_OVERCAST:
      overcast();
      break;
      
    // Golden
    case LED_GOLDEN:
      sunSet();
      break;
      
    // Snow
    case LED_SNOW:
      //clearCloud();
      whiteClouds();
      break;
      
    // Lightning
    case LED_LIGHTNING:
      lightningStorm();
      break;

    case LED_OFF:
      alloff();
      break;

    case LED_RAINBOW:
      rainbow(20);
      break;
    // Do nothing
    default:
      break;
  }
}

// Display static RGB value
void rgb(uint8_t r, uint8_t g, uint8_t b) {
#if DEBUG
  Serial.print("RGB: ");
  Serial.print(r);
  Serial.print(",");
  Serial.print(g);
  Serial.print(",");
  Serial.print(b);
#endif
  for ( int i = 0; i < 30; i++ ) {
    strip_a.setPixelColor(i, r, g, b);
    strip_b.setPixelColor(i, r, g, b);
  }
  strip_a.show();
  strip_b.show();
}

void alloff(){
    for ( int i = 0; i < 30; i++ ) {
    strip_a.setPixelColor(i, 0, 0, 0);
    strip_b.setPixelColor(i, 0 , 0, 0);
  }
  strip_a.show();
  strip_b.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip_a.numPixels(); i++) {
      strip_a.setPixelColor(i, wheel((i+j) & 255));
      strip_b.setPixelColor(i, wheel((i+j) & 255));
    }
    strip_a.show();
    strip_b.show();
    delay(wait);
  }
}

void disco() {
}
/****************************************************************
 * LED Strip - Weather Functions
 ***************************************************************/

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip_a.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return strip_b.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip_a.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return strip_b.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip_a.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return strip_b.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// Clear cloud function:
void clearCloud(){
  for (int i = 0; i < 30; i++){ //for all the LEDs
    strip_a.setPixelColor(i, 0, 0, 0); //turn off in cloud one
    strip_b.setPixelColor(i, 0, 0, 0); //turn off in cloud two
  }
  strip_a.show(); //show what was set in cloud one
  strip_b.show(); //show what was set in cloud two
}

// Bluesky function
void blueSky() {
  for(int i=0; i<30; i++) {    //for all of the LEDs 
    strip_a.setPixelColor(i, 0, 170, 175);   //set LEDs a sky blue color in cloud one
    strip_b.setPixelColor(i, 0, 170, 175);   //set LEDs a sky blue color in cloud two
  }
  strip_a.show();
  strip_b.show();
}

// White clouds function
void whiteClouds() {
  for(int i=0; i<30; i++) {   //for all of the LEDs 
    strip_a.setPixelColor(i, 100, 100, 100);  //set LEDs white-ish in cloud one
    strip_b.setPixelColor(i, 100, 100, 100);  //set LEDs white-ish in cloud one
  }
  strip_a.show();
  strip_b.show();
}


// Overcast funtion
void overcast() {
  for(int i=0; i<30; i++) {   //for all of the LEDs 
            strip_a.setPixelColor(i, 70, 64, 75);  //set LEDs grey-ish in cloud one
            strip_b.setPixelColor(i, 70, 64, 75);  //set LEDs grey-ish in cloud two
   }
   strip_a.show();
   strip_b.show();
}

// Night function
void nighttime() {
  
  switch ( night_state ) {
    
    case NIGHT_0:
      for(int i=0; i<30; i++) {    //for all of the LEDs 
         strip_a.setPixelColor(i, 0, 0, 102);  //set LEDs dark blue in cloud one
         strip_b.setPixelColor(i, 0, 0, 102);  //set LEDs dark blue in cloud two
      }
      strip_a.show();
      strip_b.show();
      night_i = 1;
      night_state = NIGHT_1;
      led_time = millis();
      break;
       
    case NIGHT_1:
      if ( (millis() - led_time) >= 50 ) {
        strip_a.setPixelColor((night_i-1), 255, 255, 255);  //set LEDs white in cloud one
        strip_b.setPixelColor((night_i-1), 255, 255, 255);  //set LEDs white in cloud two
        strip_a.show();
        strip_b.show();
        night_state = NIGHT_2;
        led_time = millis();
      }
      break;
     
    case NIGHT_2:
      if ( (millis() - led_time) >= 50 ) {
        strip_a.setPixelColor(night_i, 255, 255, 255);  //set LEDs white in cloud one
        strip_b.setPixelColor(night_i, 255, 255, 255);  //set LEDs white in cloud two
        strip_a.show();
        strip_b.show();
        night_state = NIGHT_3;
        led_time = millis();
      }
      break;
       
    case NIGHT_3:
      if ( (millis() - led_time) >= 50 ) {
        strip_a.setPixelColor((night_i+1), 255, 255, 255);  //set LEDs white in cloud one
        strip_b.setPixelColor((night_i+1), 255, 255, 255);  //set LEDs white in cloud two
        strip_a.show();
        strip_b.show();
        night_state = NIGHT_4;
        led_time = millis();
      }
      break;
      
    case NIGHT_4:
      if ( (millis() - led_time) >= TWINKLE_DELAY ) {
        strip_a.setPixelColor((night_i-1), 0, 0, 102);  //set LEDs white in cloud one
        strip_b.setPixelColor((night_i-1), 0, 0, 102);  //set LEDs white in cloud two
        strip_a.show();
        strip_b.show();
        night_state = NIGHT_5;
        led_time = millis();
      }
      break;
      
    case NIGHT_5:
      if ( (millis() - led_time) >= 50 ) {
        strip_a.setPixelColor(night_i, 0, 0, 102);  //set LEDs white in cloud one
        strip_b.setPixelColor(night_i, 0, 0, 102);  //set LEDs white in cloud two
        strip_a.show();
        strip_b.show();
        night_state = NIGHT_6;
        led_time = millis();
      }
      break;
      
    case NIGHT_6:
      if ( (millis() - led_time) >= 50 ) {
        strip_a.setPixelColor((night_i+1), 0, 0, 102);  //set LEDs white in cloud one
        strip_b.setPixelColor((night_i+1), 0, 0, 102);  //set LEDs white in cloud two
        strip_a.show();
        strip_b.show();
        night_state = NIGHT_1;
        led_time = millis();
        night_i = (night_i + 15) % 300;  // Increment/reset index
      }
      break;
      
      default:
        night_state = NIGHT_0;
        break;
  }
}

// Sun set function:
void sunSet(){  
  int i;
  for(i=0; i<6; i++) {   //for the first 100 LEDs
    strip_a.setPixelColor(i, 150, 20, 0);   //set LEDs red-ish in cloud one
    strip_b.setPixelColor(i, 150, 20, 0);   //set LEDs red-ish in cloud two
  }
  for(i=7; i<12; i++) {  //for LEDs 101 to 150
    strip_a.setPixelColor(i, 150, 90, 0);  //set LEDs orange-red in cloud one
    strip_b.setPixelColor(i, 150, 90, 0);  //set LEDs orange-red in cloud two
  }
  for(i=13; i<18; i++) {  //for LEDs 151 to 250
    strip_a.setPixelColor(i, 150, 50, 0);  //set LEDs red-orage in cloud one
    strip_b.setPixelColor(i, 150, 50, 0);  //set LEDs red-orage in cloud one
  }
  for(i=19; i<24; i++) {  //for LEDs 251 to 260
    strip_a.setPixelColor(i, 150, 50, 0);  //set LEDs red-orage in cloud one
    strip_b.setPixelColor(i, 150, 50, 0);  //set LEDs red-orage in cloud two
  }
  for(i=25; i<30; i++) {  //for LEDs 261 to the last LED
     strip_a.setPixelColor(i, wheel(((i * 256 / 300)) & 255));  //set LEDs to the first part of the color wheel (red-yellow) in cloud one
     strip_b.setPixelColor(i, wheel(((i * 256 / 300)) & 255));  //set LEDs to the first part of the color wheel (red-yellow) in cloud two
  }
    strip_a.show(); //show all the colors that were set in cloud one
    strip_b.show(); //show all the colors that were set in cloud two
}

// Lightning storm function
void lightningStorm() {

#if DEBUG
  Serial.print("Storm: ");
  Serial.println(storm_state);
#endif
  
  //Various types of lightning functions, where the first two 
  //numbers represent the area that the flash could randomly show 
  //up in, the next number is usually how long the flash is (in milisec)
  //the forth number is sometimes the size of the flash, and the
  //last is the color setting for the flash
  if ( (millis() - led_time) >= storm_delay ) {
    switch ( storm_state ) {
      case 0:
        jumpingFlash_a(50, 80, 50, strip_a.Color(255,255,255));
        break;
      case 1:
        jumpingFlash_a(50, 80, 50, strip_a.Color(255,255,255));
        break;
      case 2:
        scrollingFlash_a(20, 65, 50, 5, strip_a.Color(255,255,255));
        break;
      case 3:
        singleFlash_a(100, 200, 50, 15, strip_a.Color(255,255,255));
        Serial.println("case 3 - singleFlash");
        break;
      case 4:
        singleFlash_a(100, 200, 50, 15, strip_a.Color(255,255,255));
        break;
      case 5:
        singleFlash_a(100, 200, 50, 15, strip_a.Color(255,255,255));
        break;
      case 6:
        scrollingFlash_a(200, 250, 50, 15, strip_a.Color(255,255,255));
        break;
      case 7:
        singleFlash_a(50, 100, 50, 5, strip_a.Color(200,200,255));
        break;
      case 8:
        scrollingFlash_a(200, 250, 50, 15, strip_a.Color(255,255,255));
        break;
      case 9:
        jumpingFlash_a(100, 130, 50, strip_a.Color(255,255,255));
        break;
      case 10:
        jumpingFlash_a(100, 130, 50, strip_a.Color(255,255,255));
        break;
      case 11:
        multipleFlashs_a(20, 125, 150, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 12:
        flickerFlash_a(50, 300, 50, 25, strip_a.Color(200,200,255));
        break;
      case 13:
        multipleFlashs_a(20, 125, 150, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 14:
        scrollingFlash_a(10, 60, 100, 15, strip_a.Color(225,200,255));
        break;
      case 15:
        flickerFlash_a(75, 175, 40, 25, strip_a.Color(255,255,255));
        break;
      case 16:
        flickerFlash_a(75, 175, 40, 25, strip_a.Color(255,255,255));
        break;
      case 17:
        flickerFlash_a(75, 175, 40, 25, strip_a.Color(255,255,255));
        break;
      case 18:
        jumpingFlash_a(200, 130, 50, strip_a.Color(255,255,255));
        break;
      case 19:
        flickerFlash_a(75, 175, 40, 25, strip_a.Color(255,255,255));
        break;
      case 20:
        flickerFlash_a(50, 300, 50, 25, strip_a.Color(200,200,255));
        break;
      case 21:
        scrollingFlash_a(200, 250, 100, 10, strip_a.Color(255,255,255));
        break;
      case 22:
        wholeCloudFlash_a(40, 100, strip_a.Color(255,255,255));
        break;
      case 23:
        multipleFlashs_a(20, 125, 175, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 24:
        scrollingFlash_a(20, 65, 50, 3, strip_a.Color(255,255,255));
        break;
      case 25:
        flickerFlash_a(0, 100, 50, 50, strip_a.Color(200,200,255));
        break;
      case 26:
        flickerFlash_a(0, 100, 50, 50, strip_a.Color(200,200,255));
        break;
      case 27:
        singleFlash_a(75, 175, 40, 3, strip_a.Color(255,255,255));
        break;
      case 28:
        singleFlash_a(100, 200, 50, 15, strip_a.Color(255,255,255));
        break;
      case 29:
        scrollingFlash_a(20, 65, 50, 5, strip_a.Color(255,255,255));
        break;
      case 30:
        multipleFlashs_a(20, 125, 200, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 31:
        singleFlash_a(100, 200, 50, 30, strip_a.Color(255,255,255));
        break;
      case 32:
        flickerFlash_a(0, 100, 50, 50, strip_a.Color(200,200,255));
        break;
      case 33:
        scrollingFlash_a(200, 500, 50, 15, strip_a.Color(255,255,255));
        break;
      case 34:
        jumpingFlash_a(250, 300, 50, strip_a.Color(255,255,255));
        break;
      case 35:
        jumpingFlash_a(250, 300, 50, strip_a.Color(255,255,255));
        break;
      case 36:
        multipleFlashs_a(20, 125, 200, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 37:
        scrollingFlash_a(10, 60, 100, 7, strip_a.Color(225,200,255));
        break;
      case 38:
        singleFlash_a(75, 175, 40, 3, strip_a.Color(255,255,255));
        break;
      case 39:
        singleFlash_a(75, 175, 40, 3, strip_a.Color(255,255,255));
        break;
      case 40:
        jumpingFlash_a(150, 180, 50, strip_a.Color(255,255,255));
        break;
      case 41:
        jumpingFlash_a(50, 80, 50, strip_a.Color(255,255,255));
        break;
      case 42:
        scrollingFlash_a(250, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 43:
        flickerFlash_a(0, 100, 50, 50, strip_a.Color(200,200,255));
        break;
      case 44:
        scrollingFlash_a(250, 300, 50, 5, strip_a.Color(255,255,255));
        break;
      case 45:
        multipleFlashs_a(20, 125, 150, 300, 50, 5, strip_a.Color(200,200,255));
        break;
      case 46:
        wholeCloudFlash_a(40, 100, strip_a.Color(255,255,255));
        break;
      default:
        break;
    }
    storm_state = (storm_state + 1) % 47;
    storm_delay = random(900, 1500);
    led_time = millis();
  }
}

/****************************************************************
 * LED Strip - Lightning Functions
 ***************************************************************/

void singleFlash_a(int areaMin, int areaMax, int duration, int Size, uint32_t color) {
  int i;
  int flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_a.setPixelColor(i, 255,255,255);//color);
 }
 strip_a.show();
 delay(duration);
 for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_a.setPixelColor(i, 0, 0, 0);
 }
 strip_a.show();
}

void singleFlash_b(int areaMin, int areaMax, int duration, int Size, uint32_t color) {
  int i;
  int flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(duration);
 for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_b.setPixelColor(i, 0, 0, 0);
 }
 strip_b.show();
}

void flickerFlash_a(int areaMin, int areaMax, int duration, int Size, uint32_t color) {
  int i;
  int flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(50);
 for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_a.setPixelColor(i, 0, 0, 0);
 }
 strip_a.show();
 delay(50);
 flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(duration);
 for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_a.setPixelColor(i, 0, 0, 0);
 }
 strip_a.show();
}


void flickerFlash_b(int areaMin, int areaMax, int duration, int Size, uint32_t color) {
  int i;
  int flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(50);
 for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_b.setPixelColor(i, 0, 0, 0);
 }
 strip_b.show();
 delay(50);
 flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(duration);
 for ((i = flashStrike - Size); (i < flashStrike); i++) {
   strip_b.setPixelColor(i, 0, 0, 0);
 }
 strip_b.show();
}


void scrollingFlash_a(int areaMin, int areaMax, int duration, int Size, uint32_t color) {
  int i;
  int flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_a.setPixelColor(i, color);
    strip_a.show();
  }
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_a.setPixelColor(i, 0, 0, 0);
    strip_a.show();
  }
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_a.setPixelColor(i, color);
    strip_a.show();
  }
  delay(duration);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_a.setPixelColor(i, 0, 0, 0);
    strip_a.show();
  }
} 


void scrollingFlash_b(int areaMin, int areaMax, int duration, int Size, uint32_t color) {
  int i;
  int flashStrike = random(areaMin, areaMax);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_b.setPixelColor(i, color);
    strip_b.show();
  }
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_b.setPixelColor(i, 0, 0, 0);
    strip_b.show();
  }
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_b.setPixelColor(i, color);
    strip_b.show();
  }
  delay(duration);
  for ((i = flashStrike - Size); (i < flashStrike); i++) {
    strip_b.setPixelColor(i, 0, 0, 0);
    strip_b.show();
  }
} 


void multipleFlashs_a(int areaOneMin, int areaOneMax, int areaTwoMin, int areaTwoMax, int duration, int Size, uint32_t color){
 int i;
 int oneStrike = random(areaOneMin, areaOneMax);
 int twoStrike = random(areaTwoMin, areaTwoMax);
 for ((i = oneStrike - Size); (i < oneStrike); i++) {
   strip_a.setPixelColor(i, color);
   strip_a.show();
 }
 for ((i = twoStrike - Size); (i < twoStrike); i++) {
   strip_a.setPixelColor(i, color);
   strip_a.show();
 }
 delay(duration);
 for ((i = oneStrike - Size); (i < oneStrike); i++) {
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
 for ((i = twoStrike - Size); (i < twoStrike); i++) {
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
}


void multipleFlashs_b(int areaOneMin, int areaOneMax, int areaTwoMin, int areaTwoMax, int duration, int Size, uint32_t color){
 int i;
 int oneStrike = random(areaOneMin, areaOneMax);
 int twoStrike = random(areaTwoMin, areaTwoMax);
 for ((i = oneStrike - Size); (i < oneStrike); i++) {
   strip_b.setPixelColor(i, color);
   strip_b.show();
 }
 for ((i = twoStrike - Size); (i < twoStrike); i++) {
   strip_b.setPixelColor(i, color);
   strip_b.show();
 }
 delay(duration);
 for ((i = oneStrike - Size); (i < oneStrike); i++) {
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
 for ((i = twoStrike - Size); (i < twoStrike); i++) {
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
}


void jumpingFlash_a(int areaStart, int areaEnd, int duration, uint32_t color) {
 int i;
 for (i = areaStart; (i < areaStart + 15); i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(duration);
 for (i = areaStart; (i < areaStart + 15); i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(duration);
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
 }
 strip_a.show();
 delay(duration);
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(duration + 50);
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
}


void jumpingFlash_b(int areaStart, int areaEnd, int duration, uint32_t color) {
 int i;
 for (i = areaStart; (i < areaStart + 15); i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(duration);
 for (i = areaStart; (i < areaStart + 15); i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(duration);
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
 }
 strip_b.show();
 delay(duration);
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(duration + 50);
 for ((i = areaEnd - 15); i < areaEnd; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
}


void wholeCloudFlash_a(int durationShort, int durationLong, uint32_t color) {
  int i;
  for (i = 60; i < 75; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(durationShort);
 for (i = 60; i < 75; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
 }
    strip_a.show();
    delay(durationLong);
 for (i = 60; i < 75; i++){
   strip_a.setPixelColor(i, color);
   strip_a.show();
 }
 delay(durationShort);
 for (i = 60; i < 75; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
  for (i = 100; i < 115; i++){
   strip_a.setPixelColor(i, color);
   strip_a.show();
 }
 for (i = 100; i < 115; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
 for (i = 100; i < 115; i++){
   strip_a.setPixelColor(i, color);
   strip_a.show();
 }
 delay(durationShort);
 for (i = 100; i < 115; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
 for (i = 115; i < 130; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(durationShort);
 for (i = 115; i < 130; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
 }
 strip_a.show();
 delay(durationShort);
 for (i = 115; i < 130; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(durationShort);
 for (i = 115; i < 130; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
 for (i = 130; i < 150; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(durationShort);
 for (i = 130; i < 150; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
 }
 strip_a.show();
 delay(durationShort);
 for (i = 130; i < 150; i++){
   strip_a.setPixelColor(i, color);
 }
 strip_a.show();
 delay(durationLong);
 for (i = 130; i < 150; i++){
   strip_a.setPixelColor(i, 0, 0, 0);
   strip_a.show();
 }
}

void wholeCloudFlash_b(int durationShort, int durationLong, uint32_t color) {
  int i;
  for (i = 60; i < 75; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(durationShort);
 for (i = 60; i < 75; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
 }
    strip_b.show();
    delay(durationLong);
 for (i = 60; i < 75; i++){
   strip_b.setPixelColor(i, color);
   strip_b.show();
 }
 delay(durationShort);
 for (i = 60; i < 75; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
  for (i = 100; i < 115; i++){
   strip_b.setPixelColor(i, color);
   strip_b.show();
 }
 for (i = 100; i < 115; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
 for (i = 100; i < 115; i++){
   strip_b.setPixelColor(i, color);
   strip_b.show();
 }
 delay(durationShort);
 for (i = 100; i < 115; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
 for (i = 115; i < 130; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(durationShort);
 for (i = 115; i < 130; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
 }
 strip_b.show();
 delay(durationShort);
 for (i = 115; i < 130; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(durationShort);
 for (i = 115; i < 130; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
 for (i = 130; i < 150; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(durationShort);
 for (i = 130; i < 150; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
 }
 strip_b.show();
 delay(durationShort);
 for (i = 130; i < 150; i++){
   strip_b.setPixelColor(i, color);
 }
 strip_b.show();
 delay(durationLong);
 for (i = 130; i < 150; i++){
   strip_b.setPixelColor(i, 0, 0, 0);
   strip_b.show();
 }
}
