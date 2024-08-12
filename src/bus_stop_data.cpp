/*
 * Private includes
 */
#include "bus_stop_data.h"

/*
 * External includes
 */
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <string.h>

const char *api_url_inbound = "https://mpolden.no/atb/v2/departures/42282?direction=inbound";
const char *api_url_outbound = "https://mpolden.no/atb/v2/departures/42282?direction=outbound";

void fetch_bus_stop_data(bus_stop_data_t *bus_stop_data, direction_t direction)
{
  HTTPClient http;

  memset(bus_stop_data, 0, sizeof(*bus_stop_data));
  bus_stop_data->direction = direction;
  bus_stop_data->error = false;

  if (direction == INBOUND)
    http.begin(api_url_inbound);
  else
    http.begin(api_url_outbound);

  int status = http.GET();

  if (status != 200)
  {
    bus_stop_data->error = true;
    return;
  }

  const char *payload = http.getString().c_str();
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    bus_stop_data->error = true;
    return;
  }

  for (int i = 0; i < NUM_BUSSES; i++)
  {
    const char *dest = doc["departures"][i]["destination"];
    const char *space = strchr(dest, ' ');
    size_t dest_size = strlen(dest);

    if (space != NULL)
      dest_size = (int)(space - dest);

    // special case for 'A Strindheim'
    if (dest[0] == 'A')
      dest_size = 12;

    memcpy(bus_stop_data->busses[i].dest, dest, dest_size);

    const char *dots = "...";
    if (space != NULL)
      strcat(bus_stop_data->busses[i].dest, dots);

    const char *line = doc["departures"][i]["line"];
    size_t line_size = strlen(line);

    // align bus numbers
    if (line_size == 1)
    {
      bus_stop_data->busses[i].line[0] = ' ';
      memcpy(bus_stop_data->busses[i].line + 1, line, line_size);
    }
    else
    {
      memcpy(bus_stop_data->busses[i].line, line, line_size);
    }
  }

  http.end();
}

void oled_write_loading_screen(direction_t direction, Adafruit_SSD1306 *display)
{
  display->clearDisplay();

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F(direction ? "Fra sentrum" : "Mot sentrum"));

  display->drawLine(0, display->height() / 4 - 1, display->width(), display->height() / 4 - 1, SSD1306_WHITE);

  display->drawCircle(display->width() / 2, display->height() / 2 + 10, 16, SSD1306_WHITE);

  display->display();
}

void oled_write_bus_stop_data(bus_stop_data_t *bus_stop_data, Adafruit_SSD1306 *display)
{
  display->clearDisplay();

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);

  display->setCursor(0, 0);
  display->println(F(bus_stop_data->direction ? "Fra sentrum" : "Mot sentrum"));

  display->drawLine(0, display->height() / 4 - 1, display->width(), display->height() / 4 - 1, SSD1306_WHITE);

  for (int i = 0; i < NUM_BUSSES; i++)
  {
    display->setCursor(0, display->height() / 4 + 8 + i * display->height() / 4);

    char bus_data_string[32];
    sprintf(bus_data_string, "%s %s", bus_stop_data->busses[i].line, bus_stop_data->busses[i].dest);

    display->println(F(bus_data_string));
  }

  display->display();
}
