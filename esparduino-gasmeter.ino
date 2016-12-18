#include "confparams.h"
#include "leds.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


WiFiClient espClient;

boolean checkWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  Serial.println(F("Wifi disconnected, connecting..."));

  switchLed(HIGH);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (tries >= 20) {
      // No connection after 10 seconds, error.
      Serial.println(F("Wifi connection unsuccessful..."));
      sos();
      delay(500);
      return false;
    }
    tries++;

    toggleLed();
    delay(500);
  }

  // connection succeeded
  Serial.println(F("Wifi connection successful."));
  blinkLed(100, 5);
  return true;
}

void sendGasMeterEvent() {
  Serial.println(F("Sending gas meter event."));
  HTTPClient http;
  http.begin(HTTP_URL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("HTTP request successful.");
  }
  else {
    Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
}

/**
   Rising edge detected, (pulled up)
*/
void handleRisingEdge() {
  switchLed(LOW);
}

/**
   Falling edge detected, trigger web call.
*/
void handleFallingEdge() {
  switchLed(HIGH);
  sendGasMeterEvent();
}

int pinState = LOW;

/**

*/
void checkPinStateChange(int newPinState) {
  if (newPinState == pinState) {
    return;
  }
  pinState = newPinState;
  switch (pinState) {
    case HIGH:
      handleRisingEdge();
      break;
    case LOW:
      handleFallingEdge();
      break;
    default:
      /* do I smell fish? */
      break;
  }
}

void setup() {
  pinMode(COUNT_PIN, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  randomSeed(micros());
  Serial.begin(115200);
  Serial.println(F(""));
  reconnectWifi();
}

void reconnectWifi() {
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  if (!checkWifi()) {
    reconnectWifi();
    delay(500);
    return;
  }
  int newPinState = digitalRead(COUNT_PIN);
  checkPinStateChange(newPinState);
  delay(200); // sleep for button bounces
}
