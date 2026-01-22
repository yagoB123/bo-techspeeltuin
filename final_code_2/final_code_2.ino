// Includes


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

// State
bool playing = true;

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

// Main code

void setup() {
  Serial.begin(9600);
  setup_button();
  setup_led();
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

