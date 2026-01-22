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

// DFPlayer 
DFRobotDFPlayerMini myDFPlayer;
const bool dfDebug = true;
const int  dfVolume = 10;

// State
bool playing = true;
int  selectedSong = 1;

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
  myDFPlayer.play(selectedSong);  //Play the first mp3
  if ( dfDebug ) Serial.println("First song started");
}

// Main code

void setup() {
  Serial.begin(9600);
  setup_button();
  setup_led();
  setup_dfplayer();
}


void loop() {
  if ( handle_button() ) {
    playing = !playing;

    if ( playing ) {
      Serial.println("play");
    } else {
      Serial.println("pause");
    }
  }
  handle_led();
}

