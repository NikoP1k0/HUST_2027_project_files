#include "LED_functions.h"

// Variables for blinking LEDs
int count = 0;
unsigned long previousMillisLEDs = 0;
const int blinkInterval = 500;
bool ledState = LOW;
bool brakeLedState = LOW;

int left_blinker;
int right_blinker;
int hazard_light;
int brake_light;
int horn;

void setupLEDs(){

  // Blinkers setup
  pinMode(LEFT_BLINKER, OUTPUT);
  digitalWrite(LEFT_BLINKER, LOW);
  pinMode(RIGHT_BLINKER, OUTPUT);
  digitalWrite(RIGHT_BLINKER, LOW);
  pinMode(BRAKE_LIGHT, OUTPUT);
  digitalWrite(BRAKE_LIGHT, LOW);
  pinMode(READ_BRAKE_LIGHT, INPUT);
}

void blinkLEDs() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisLEDs >= blinkInterval) {
    previousMillisLEDs = currentMillis;
    ledState = !ledState;

    // Blink LEDS
    if (left_blinker || hazard_light) {
      digitalWrite(LEFT_BLINKER, ledState);
      Serial.println("LEFT BLINKER or hazard");
    } else
      digitalWrite(LEFT_BLINKER, LOW);

    if (right_blinker|| hazard_light) {
      digitalWrite(RIGHT_BLINKER, ledState);
      Serial.println("RIGHT BLINKER or hazard");
    } else
      digitalWrite(RIGHT_BLINKER, LOW);

    if (brake_light || digitalRead(READ_BRAKE_LIGHT)) {
      digitalWrite(BRAKE_LIGHT, HIGH);
      Serial.println("BRAKE LIGHT");
    } else
      digitalWrite(BRAKE_LIGHT, LOW);
  }
}
