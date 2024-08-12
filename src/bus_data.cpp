/*
 * Private includes
 */
#include "bus_data.h"

/*
 * External includes
 */
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char *api_url_inbound = "https://mpolden.no/atb/v2/departures/42282?direction=inbound";
const char *api_url_outbound = "https://mpolden.no/atb/v2/departures/42282?direction=outbound";

bus_stop_data_t fetch_bus_data(direction_t direction)
{
  HTTPClient http;
  bus_stop_data_t result;

  if (direction == INBOUND)
    http.begin(api_url_inbound);
  else
    http.begin(api_url_outbound);

  int status = http.GET();

  if (status != 200)
  {
    result.error = true;
    return result;
  }

  String payload = http.getString();
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    result.error = true;
    return result;
  }

  return result;
}
