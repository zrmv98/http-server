#pragma once

#include <string>

namespace weather_parser {

struct WeatherData {
    std::string city;
    std::string date;

    double current_temp_c;
    double min_temp_c;
    double max_temp_c;

    std::string condition;
};

WeatherData ParseWeatherJson(const std::string& weather_json);

} // namespace weather_parser