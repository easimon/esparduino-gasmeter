#ifndef LEDFUNCTIONS_H
#define LEDFUNCTIONS_H

/**
   Inverted HIGH/LOW logic
*/
void updateLed();

void switchLed(int state);

void toggleLed();

void blinkLed(int delayms, int times);

void sos();

#endif
