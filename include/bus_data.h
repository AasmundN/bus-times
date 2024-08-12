#ifndef __BUS_DATA__
#define __BUS_DATA__

#include <Arduino.h>

typedef enum
{
  INBOUND,
  OUTBOUND,
} direction_t;

typedef struct
{
  int departure_time;
  String line;
  String dest;
} bus_t;

typedef struct
{
  direction_t direction;
  bus_t busses[3];
  bool error;
} bus_stop_data_t;

bus_stop_data_t fetch_bus_data(direction_t direction);

#endif // __BUS_DATA__