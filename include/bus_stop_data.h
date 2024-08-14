#ifndef __BUS_DATA__
#define __BUS_DATA__

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#define NUM_BUSSES 3
#define NUM_BUS_STOPS 2

#define DESTINATION_BUFF_SIZE 20
#define LINE_BUFF_SIZE 3
#define DEPARTURE_TIME_BUFF_SIZE 4

typedef enum
{
  EAST,
  WEST,
  NONE,
} bus_stop_t;

typedef enum
{
  OUTBOUND,
  INBOUND,
} direction_t;

typedef struct
{
  char departure_time[DEPARTURE_TIME_BUFF_SIZE];
  char dest[DESTINATION_BUFF_SIZE];
  char line[LINE_BUFF_SIZE];
  direction_t direction;
} bus_data_t;

typedef struct
{
  bus_data_t busses[NUM_BUS_STOPS][NUM_BUSSES];
  bool error;
} bus_stop_data_t;

void fetch_bus_stop_data(bus_stop_data_t *bus_stop_data);

void oled_write_loading_screen(bus_stop_t bus_stop, Adafruit_SSD1306 *display);

void oled_write_bus_stop_data(bus_stop_t bus_stop, bus_stop_data_t *bus_stop_data, Adafruit_SSD1306 *display);

#endif // __BUS_DATA__