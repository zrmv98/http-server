#pragma once

#include "http_message.h"

namespace weather_handler {

simple_http_server::HttpResponse HandleWeatherRequest(const simple_http_server::HttpRequest& request);

}  // namespace weather_handler