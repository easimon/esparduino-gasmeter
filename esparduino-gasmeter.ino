#include "confparams.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int ledState = LOW;
WiFiClient espClient;
PubSubClient client(espClient);
const int clientIdLen = strlen(MQTT_CLIENT_ID_PREFIX) + 5;
char clientId[clientIdLen] = MQTT_CLIENT_ID_PREFIX;
unsigned long lastHeartbeatMillis = 0;

/**
 * Inverted HIGH/LOW logic
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

void checkWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 0;
  switchLed(HIGH);

  while (WiFi.status() != WL_CONNECTED) {
    tries++;
    toggleLed();

    delay(500);
    if (tries >= 20) {
      // No connection after 10 seconds, error.
      while (true) {
        sos();
        delay(500);
      }
    }

  }
  // connection succeeded
  blinkLed(100, 5);
} 

void generateClientId() {
  String suffix = String(random(0xffff), HEX);
  strncpy(clientId + clientIdLen - 5, suffix.c_str(), 5);
}

void checkMqtt() {
  while (!client.connected()) {
    generateClientId();
    if (client.connect(clientId)) {
      sendHeartbeat(millis());
    } 
    else {
      // display MQTT Failure: 2 times SOS, retry after 10 seconds
      sos();
      delay(500);
      sos();
      delay(9500-(3600*2));
    }
  }
}

void checkHeartbeat() {
  // almost overflow safe -- may decide to send two heartbeats directly after each other when millis overflow.
  unsigned long nowMillis = millis();
  unsigned long diff = nowMillis - lastHeartbeatMillis;
  if (diff > MQTT_HEARTBEAT_INTERVAL) {
    sendHeartbeat(nowMillis);
  }
}

void sendHeartbeat(unsigned long nowMillis) {
  client.publish(MQTT_TOPIC_HEARTBEAT, clientId);
  lastHeartbeatMillis = nowMillis;
}

void sendGasMeterEvent() {
  String message = String(MQTT_GASMETER_EVENT_TEXT_BEFORE_CLIENT_ID);
  message.concat(clientId);
  message.concat(MQTT_GASMETER_EVENT_TEXT_AFTER_CLIENT_ID);
  client.publish(MQTT_TOPIC_GASMETER, message.c_str());
}

/**
 * Rising edge detected, (pulled up)
 */
void handleRisingEdge() {
  switchLed(LOW);
}

/**
 * Falling edge detected, trigger web call.
 */
void handleFallingEdge() {
  switchLed(HIGH);
  sendGasMeterEvent();
}

int pinState = LOW;

/**
 *
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
  client.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {
  checkWifi();
  checkMqtt();
  checkHeartbeat();
  int newPinState = digitalRead(COUNT_PIN);
  checkPinStateChange(newPinState);
  delay(200); // sleep for button bounces
}
