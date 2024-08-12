#include "credentials.h"
#include <Arduino.h>
#include <esp_wifi.h>
#include <wifi.h>

#define LED_BUILTIN_PIN 2
#define SW_PIN 23

#define BAUD_RATE 9600
#define SW_DEBOUNCE_TRESHOLD 5

hw_timer_t *swTimer = NULL;
volatile uint8_t swDebounceCount = 0;

void IRAM_ATTR swTimerISR()
{
  if (digitalRead(SW_PIN) == HIGH)
  {
    swDebounceCount = 0;
    return;
  }

  if (++swDebounceCount == SW_DEBOUNCE_TRESHOLD)
    digitalWrite(LED_BUILTIN_PIN, !digitalRead(LED_BUILTIN_PIN));
}

void setup()
{
  Serial.begin(BAUD_RATE);

  /*
   * Configure pins
   */
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  /*
   * Connect to WiFi
   */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
    ;

  digitalWrite(LED_BUILTIN_PIN, HIGH);

  /*
   * Configure SW timer
   */
  const uint16_t PRESCALER = 80;
  const uint8_t TIMER_ID = 0;
  swTimer = timerBegin(TIMER_ID, PRESCALER, true);

  timerAttachInterrupt(swTimer, swTimerISR, true);

  const uint16_t ALARM_VALUE_us = 10000;
  timerAlarmWrite(swTimer, ALARM_VALUE_us, true);
  timerAlarmEnable(swTimer);
}

void loop()
{
}
