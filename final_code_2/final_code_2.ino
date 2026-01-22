// Includes for DFPlayer
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))  // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/10, /*tx =*/11);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

// Includes for color sensor
#include <Wire.h>
#include "Adafruit_TCS34725.h"


// Constants and globals

// Button
const int buttonPin = 7;
const int debounceDelay = 125;
const bool btnDebug = false;
int btnLastHigh = 0;
bool btnChanged = true;

// Led
const int ledPin = 3;
const int ledBlinkInterval = 500;
bool ledOn = false;
int ledLastChange = 0;

// Songs
const int firstSong = 5;
const int redSong = 2;
const int orangeSong = 6;
const int yellowSong = 3;
const int greenSong = 4;
const int blueSong = 1;

// DFPlayer 
DFRobotDFPlayerMini myDFPlayer;
const bool dfDebug = true;
const int  dfVolume = 10;
int  dfSongPlaying = firstSong;
bool dfIsPlaying = false;

// Color Sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_600MS, TCS34725_GAIN_16X);

struct colorCalibration {
  unsigned int blackValue;
  unsigned int whiteValue;
};

colorCalibration redCal, greenCal, blueCal;
const bool tcsDebug = true;

// State
bool playing = true;
int  songSelected = dfSongPlaying;
bool needColorCheck = false;

// Button code

void setup_button() {
  // Button is pullup, so normally high
  pinMode(buttonPin, INPUT_PULLUP);
}

bool handle_button() {
  int btnState = digitalRead(buttonPin);
  if ( btnState == HIGH ) {
    // HIGH betekent knop niet ingedrukt
    btnLastHigh = millis();
    if ( btnDebug ) Serial.print("High ");
    btnChanged = false; // Mogelijk krijgen we een klik
  } else {
    // Low is knop ingedrukt
    if ( btnDebug ) Serial.print("low  ");
    if ( ( ! btnChanged ) && millis() - btnLastHigh > debounceDelay ) {
      // Alds ee knop langer dan debounceDelay is ingedruk hebben we een klik
      btnChanged = true;
      return true;
    }
  }
  return false;
}

// Led functions

void setup_led() {
  pinMode(ledPin, OUTPUT); 
}

void handle_led() {
  if ( playing ) {
    // Als we spelen, led constant aan
    digitalWrite(ledPin, HIGH);
    ledOn = true;
    ledLastChange = millis();
  } else {
    // Als we pauze staan, knipperen
    if ( millis() - ledLastChange >= ledBlinkInterval ) {
      ledOn = ! ledOn;
      if ( ledOn ) {
        digitalWrite(ledPin, HIGH);
      } else {
        digitalWrite(ledPin, LOW);
      }
      ledLastChange = millis();
    }
  }
}

// DFPlayer

void setup_dfplayer() {
  #if (defined ESP32)
    FPSerial.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
  #else
    FPSerial.begin(9600);
  #endif

  if ( dfDebug ) Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  while ( ! myDFPlayer.begin(FPSerial, true, true) ) {
    if ( dfDebug ) {
      Serial.println("Unable to begin:");
      Serial.println("1.Please recheck the connection!");
      Serial.println("2.Please insert the SD card!");    
    }
  }
  if ( dfDebug ) Serial.println("DFPlayer Mini online.");
  myDFPlayer.volume(dfVolume);  //Set volume value. From 0 to 30
  if ( dfDebug ) Serial.println("DFPlayer volume set.");
  myDFPlayer.play(dfSongPlaying);  //Play the first mp3
  dfIsPlaying = true;
  if ( dfDebug ) Serial.println("First song started");
}

void handle_dfplayer() {
  if ( dfIsPlaying != playing ) {
    if ( playing ) {
      myDFPlayer.start();
    } else {
      myDFPlayer.pause();
    }
    dfIsPlaying = playing;
  }
}

// Color sensor
void setup_colorsensor() {
  if (tcs.begin()) {
    if ( tcsDebug) Serial.println("Found TCS3472 sensor");
  } else {
    if ( tcsDebug ) Serial.println("No TCS34725 found ... check your connections");
    while (1)
      ;  // halt!
  }

  // We calibrated this with white and black.
  redCal.blackValue = 8198;
  redCal.whiteValue = 65535;
  greenCal.blackValue = 11269;
  greenCal.whiteValue = 65535;
  blueCal.blackValue = 9329;
  blueCal.whiteValue = 65535;

}

int sense_color() {
  unsigned int r, g, b, c;  // raw values of r,g,b,c as read by TCS3472
  // Variables used to hold RGB values between 0 and 255
  int redValue;
  int greenValue;
  int blueValue;
  int clearValue;

  tcs.getRawData(&r, &g, &b, &c);

  char buffer[40];
  sprintf(buffer, "R: %d G: %d B: %d C: %d", r, g, b, c);
  Serial.println(buffer);

  delay(50);

  redValue = RGBmap(r, redCal.blackValue, redCal.whiteValue, 0, 255);
  greenValue = RGBmap(g, greenCal.blackValue, greenCal.whiteValue, 0, 255);
  blueValue = RGBmap(b, blueCal.blackValue, blueCal.whiteValue, 0, 255);

  sprintf(buffer, "R: %d G: %d B: %d", redValue, greenValue, blueValue);
  if ( tcsDebug) Serial.println(buffer);

  if ( blueValue > 200 ) {
    // Bal is blauw als blauw > 200"
    if ( tcsDebug) Serial.println("Blauw");
    return blueSong;
  } else if ( redValue < 150 && greenValue > 200 ) {
    // Groen
    if ( tcsDebug) Serial.println("Groen");
    return greenSong;
  } else if ( redValue > 200 ) {
    // Geel, Rood of Oranje
    if ( greenValue < 200 ) {
      // Rood
      if ( tcsDebug) Serial.println("rood");
      return redSong;
    } else {
      // Geel of oranje
      if ( blueValue < 100 ) {
        if ( tcsDebug) Serial.println("oranje");
        return orangeSong;
      } else {
        if ( tcsDebug) Serial.println("geel");
        return yellowSong;
      }
    }
  }
  if ( tcsDebug) Serial.println("Iets anders");
  return firstSong;
}

int RGBmap(unsigned int x, unsigned int inlow, unsigned int inhigh, int outlow, int outhigh) {
  float flx = float(x);
  float fla = float(outlow);
  float flb = float(outhigh);
  float flc = float(inlow);
  float fld = float(inhigh);

  float res = ((flx - flc) / (fld - flc)) * (flb - fla) + fla;

  return int(res);
}


// Main code

void setup() {
  Serial.begin(9600);
  setup_button();
  setup_led();
  setup_dfplayer();
  setup_colorsensor();
}


void loop() {
  if ( handle_button() ) {
    playing = !playing;

    if ( playing ) {
      Serial.println("play");
      needColorCheck = true;
    } else {
      Serial.println("pause");
    }
  }
  handle_led();
  handle_dfplayer();
  if ( needColorCheck ) {
    sense_color();
    needColorCheck = false;
  }
}

