/*
 * Private includes
 */
#include "bus_stop_data.h"

/*
 * External includes
 */
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MINUTE_s 60

const char *api_url_bus_stop_data = "https://mpolden.no/atb/v2/departures/42282";

typedef enum
{
  THREE = 3,
  TWELVE = 12,
  FOURTEEN = 14,
  FIFTEEN = 15,
  FORTY_THREE = 43,
} line_t;

void parse_departure_time(char departure_time_diff[4], const char *datetime_string)
{
  struct tm departure_time;

  strptime(datetime_string, "%Y-%m-%dT%H:%M:%S.000", &departure_time);

  time_t departure_time_unix = mktime(&departure_time);

  time_t now;
  time(&now);

  int time_diff = (int)difftime(departure_time_unix, now);

  if (time_diff < 1 * MINUTE_s)
  {
    sprintf(departure_time_diff, "<1m");
  }
  else if (time_diff < 60 * MINUTE_s)
  {
    sprintf(departure_time_diff, "%dm", time_diff / MINUTE_s);
  }
  else
  {
    sprintf(departure_time_diff, ">1h");
  }
}

bus_stop_t bus_data_to_bus_stop(line_t line, direction_t direction)
{
  switch (line)
  {
  case THREE:
    return direction == INBOUND ? EAST : WEST;

  case TWELVE:
    return direction == INBOUND ? EAST : WEST;

  case FOURTEEN:
    return direction == INBOUND ? WEST : EAST;

  case FORTY_THREE:
    return direction == INBOUND ? WEST : EAST;

  default:
    break;
  }

  return NONE;
}

void parse_bus_data(bus_data_t *bus_data, JsonDocument *bus_data_json, uint8_t departure_index)
{
  const direction_t direction = (direction_t)(bool)(*bus_data_json)["departures"][departure_index]["isGoingTowardsCentrum"];
  bus_data->direction = direction;

  const char *dest = (*bus_data_json)["departures"][departure_index]["destination"];
  const char *space = strchr(dest, ' ');
  size_t dest_size = strlen(dest);

  if (space != NULL)
    dest_size = (int)(space - dest);

  // special case for 'A Strindheim'
  if (dest[0] == 'A')
    dest_size = 12;

  memcpy(bus_data->dest, dest, dest_size);

  const char *dots = "...";
  if (space != NULL)
    strcat(bus_data->dest, dots);

  const char *line = (*bus_data_json)["departures"][departure_index]["line"];
  size_t line_size = strlen(line);

  // align bus numbers
  if (line_size == 1)
  {
    bus_data->line[0] = ' ';
    memcpy(bus_data->line + 1, line, line_size);
  }
  else
  {
    memcpy(bus_data->line, line, line_size);
  }

  const char *departure_time = (*bus_data_json)["departures"][departure_index]["scheduledDepartureTime"];

  char departure_time_diff_now[4];
  parse_departure_time(departure_time_diff_now, departure_time);

  size_t departure_time_size = strlen(departure_time_diff_now);

  memcpy(bus_data->departure_time, departure_time_diff_now, departure_time_size);
}

void fetch_bus_stop_data(bus_stop_data_t *bus_stop_data)
{
  HTTPClient http;

  memset(bus_stop_data, 0, sizeof(*bus_stop_data));

  http.begin(api_url_bus_stop_data);

  int status = http.GET();

  if (status != 200)
  {
    bus_stop_data->error = true;
    return;
  }

  const char *payload = http.getString().c_str();

  http.end();

  JsonDocument bus_data_json;

  DeserializationError error = deserializeJson(bus_data_json, payload);

  if (error)
  {
    bus_stop_data->error = true;
    return;
  }

  bus_data_t bus_data;

  uint8_t departure_index = 0, east_index = 0, west_index = 0;

  while (east_index < NUM_BUSSES || west_index < NUM_BUSSES)
  {
    memset(&bus_data, 0, sizeof(bus_data));
    parse_bus_data(&bus_data, &bus_data_json, departure_index++);

    const bus_stop_t bus_stop = bus_data_to_bus_stop((line_t)atoi(bus_data.line), bus_data.direction);

    if (bus_stop == EAST && east_index < NUM_BUSSES)
    {
      memcpy(&(bus_stop_data->busses[bus_stop][east_index++]), &bus_data, sizeof(bus_data));
    }
    else if (bus_stop == WEST && west_index < NUM_BUSSES)
    {
      memcpy(&(bus_stop_data->busses[bus_stop][west_index++]), &bus_data, sizeof(bus_data));
    }
  }
}

void oled_write_loading_screen(bus_stop_t bus_stop, Adafruit_SSD1306 *display)
{
  display->clearDisplay();

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F(bus_stop ? "Voll Studentby West" : "Voll Studentby East"));

  display->drawLine(0, display->height() / 4 - 1, display->width(), display->height() / 4 - 1, SSD1306_WHITE);

  display->drawCircle(display->width() / 2, display->height() / 2 + 10, 16, SSD1306_WHITE);

  display->display();
}

void oled_write_bus_stop_data(bus_stop_t bus_stop, bus_stop_data_t *bus_stop_data, Adafruit_SSD1306 *display)
{
  display->clearDisplay();

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);

  display->setCursor(0, 0);
  display->println(F(bus_stop ? "Voll Studentby West" : "Voll Studentby East"));

  display->drawLine(0, display->height() / 4 - 1, display->width(), display->height() / 4 - 1, SSD1306_WHITE);

  for (int i = 0; i < NUM_BUSSES; i++)
  {
    display->setCursor(0, display->height() / 4 + 8 + i * display->height() / 4);

    char bus_data_string[32];
    sprintf(bus_data_string, "%s %s %s", bus_stop_data->busses[bus_stop][i].line, bus_stop_data->busses[bus_stop][i].dest, bus_stop_data->busses[bus_stop][i].departure_time);

    display->print(F(bus_data_string));
  }

  display->display();
}
