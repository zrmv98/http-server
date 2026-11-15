#include <sstream>
//#include <iostream>

#include "weather_parser.h"
#include "json.h"

namespace weather_parser {

WeatherData ParseWeatherJson(const std::string& weather_json) {
    WeatherData result;

    std::istringstream input(weather_json);

    json::Document document = json::Load(input);
    const json::Dict& root = document.GetRoot().AsMap();

    const json::Dict& location = root.at("location").AsMap();
    result.city = location.at("name").AsString();

    const json::Dict& current = root.at("current").AsMap();
    result.current_temp_c = current.at("temp_c").AsDouble();
    result.condition = current.at("condition").AsMap().at("text").AsString();

    const json::Dict& forecast = root.at("forecast").AsMap();
    const json::Array& forecastday = forecast.at("forecastday").AsArray();

    //std::cout << "Forecast days count: " << forecastday.size() << '\n';
    if (forecastday.empty()) {
        throw std::invalid_argument("Forecast is not available for the requested date\n");
    }

    const json::Dict& forecast_day = forecastday.at(0).AsMap();
    result.date = forecast_day.at("date").AsString();

    const json::Dict& day = forecast_day.at("day").AsMap();
    result.min_temp_c = day.at("mintemp_c").AsDouble();
    result.max_temp_c = day.at("maxtemp_c").AsDouble();

    return result;
}

} // namespace weather_parser