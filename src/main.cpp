/*
 * Private includes
 */
#include "bus_stop_data.h"
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
 * Additional data
 */
#define SW_DEBOUNCE_TRESHOLD 5

volatile bus_stop_t current_bus_stop = EAST;
bus_stop_data_t bus_stop_data;

volatile bool refresh_data = true;
volatile bool refresh_screen = true;
volatile uint8_t sw_debounce_count = 0;

hw_timer_t *sw_timer = NULL;
hw_timer_t *data_timer = NULL;

/*
 * SW timer interrupt callback
 */
void IRAM_ATTR sw_timer_isr()
{
  if (digitalRead(SW_PIN) == HIGH)
  {
    sw_debounce_count = 0;
    return;
  }

  if (++sw_debounce_count != SW_DEBOUNCE_TRESHOLD)
    return;

  if (current_bus_stop == EAST)
    current_bus_stop = WEST;
  else
    current_bus_stop = EAST;

  timerWrite(data_timer, 0);
  refresh_screen = true;
}

/*
 * DATA refresh timer interrupt callback
 */
void IRAM_ATTR data_timer_isr()
{
  refresh_data = true;
}

void setup()
{
  Serial.begin(9600);

  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  /*
   * Init OLED screen
   */
  while (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    ;
  oled_write_loading_screen(current_bus_stop, &display);

  /*
   * Connect WiFi
   */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
    ;
  digitalWrite(LED_BUILTIN_PIN, HIGH);

  /*
   * Setup SW and data refresh hardware timers
   */
  const uint16_t PRESCALER = 80;

  sw_timer = timerBegin(0, PRESCALER, true);
  data_timer = timerBegin(1, PRESCALER, true);

  timerAttachInterrupt(sw_timer, sw_timer_isr, true);
  timerAttachInterrupt(data_timer, data_timer_isr, true);

  const uint64_t SW_ALARM_VALUE_us = 10000;
  const uint64_t DATA_ALARM_VALUE_us = 60000000;

  timerAlarmWrite(sw_timer, SW_ALARM_VALUE_us, true);
  timerAlarmWrite(data_timer, DATA_ALARM_VALUE_us, true);

  timerAlarmEnable(sw_timer);
  timerAlarmEnable(data_timer);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN_PIN, LOW);
    abort();
  }

  if (refresh_data)
  {
    fetch_bus_stop_data(&bus_stop_data);

    if (!bus_stop_data.error)
      refresh_data = false;
  }

  if (refresh_screen)
  {
    oled_write_bus_stop_data(current_bus_stop, &bus_stop_data, &display);
    refresh_screen = false;
  }
}