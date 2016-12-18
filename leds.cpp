#include "Arduino.h"

int ledState = LOW;

/**
   Inverted HIGH/LOW logic
*/
void updateLed() {
  if (ledState == HIGH) {
    digitalWrite(BUILTIN_LED, LOW);
  }
  else {
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void switchLed(int state) {
  ledState = state;
  updateLed();
}

void toggleLed() {
  ledState = (ledState == HIGH) ? LOW : HIGH;
  updateLed();
}

void blinkLed(int delayms, int times) {
  for (int i = 0; i < times; i++) {
    switchLed(HIGH);
    delay(delayms);
    switchLed(LOW);
    delay(delayms);
  }
}

void sos() {
  blinkLed(150, 3); // 2 * 3 * 150 = 900ms
  blinkLed(300, 3); // 2 * 3 * 300 = 1800ms
  blinkLed(150, 3); // 2 * 3 * 150 = 900ms  -> 3600ms for one SOS
}

