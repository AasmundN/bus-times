#include "credentials.h"
#include <Arduino.h>
#include <esp_wifi.h>
#include <wifi.h>

#define LED_BUILTIN 2
#define REQUEST_FREQ 10000
#define BAUD_RATE 9600

unsigned long lastRequest = 0;

void setup()
{
  Serial.begin(BAUD_RATE);

  // configure built in LED
  pinMode(LED_BUILTIN, OUTPUT);

  // connect to WiFi
  Serial.println("Connecting to WiFi...");

  // wifi credentials (ssid and password) must be defined in a credentials.h file
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // wait till connection succeeds
  while (WiFi.status() != WL_CONNECTED)
    ;

  // successfully connected to WiFi
  Serial.println("Connected to WiFi!");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
}