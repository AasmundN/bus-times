#ifndef __BUS_DATA__
#define __BUS_DATA__

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#define NUM_BUSSES 3
#define DESTINATION_BUFF_SIZE 20
#define LINE_BUFF_SIZE 3

typedef enum
{
  INBOUND,
  OUTBOUND,
} direction_t;

typedef struct
{
  int departure_time;
  char dest[DESTINATION_BUFF_SIZE];
  char line[LINE_BUFF_SIZE];
} bus_t;

typedef struct
{
  direction_t direction;
  bus_t busses[NUM_BUSSES];
  bool error;
} bus_stop_data_t;

void fetch_bus_stop_data(bus_stop_data_t *bus_stop_data, direction_t direction);

void oled_write_loading_screen(direction_t direction, Adafruit_SSD1306 *display);

void oled_write_bus_stop_data(bus_stop_data_t *bus_stop_data, Adafruit_SSD1306 *display);

#endif // __BUS_DATA__