/*
 * Private includes
 */
#include "Wire.h"
#include "bus_data.h"
#include "credentials.h"

/*
 * External includes
 */
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <wifi.h>

/*
 * Pin definitions
 */
#define LED_BUILTIN_PIN 2
#define SW_PIN 23

/*
 * OLED screen config
 */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*
 * SW hardware timer
 */
#define SW_DEBOUNCE_TRESHOLD 5

hw_timer_t *sw_timer = NULL;
volatile uint8_t sw_debounce_count = 0;

/*
 * SW timer interrupt callback
 */
void IRAM_ATTR swTimerISR()
{
  if (digitalRead(SW_PIN) == HIGH)
  {
    sw_debounce_count = 0;
    return;
  }

  if (++sw_debounce_count == SW_DEBOUNCE_TRESHOLD)
    digitalWrite(LED_BUILTIN_PIN, !digitalRead(LED_BUILTIN_PIN));
}

void setup()
{
  Serial.begin(9600);

  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  /*
   * Init OLED screen
   */
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();

  /*
   * Connect WiFi
   */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
    ;
  digitalWrite(LED_BUILTIN_PIN, HIGH);

  /*
   * Setup SW hardware timer
   */
  const uint16_t PRESCALER = 80;
  const uint8_t TIMER_ID = 0;
  sw_timer = timerBegin(TIMER_ID, PRESCALER, true);

  timerAttachInterrupt(sw_timer, swTimerISR, true);

  const uint16_t ALARM_VALUE_us = 10000;
  timerAlarmWrite(sw_timer, ALARM_VALUE_us, true);
  timerAlarmEnable(sw_timer);

  fetch_bus_data(INBOUND);
}

void loop()
{
}
