#include <stdexcept>
#include <unordered_map>
#include <string>
#include <cctype>
#include <sstream>
#include <iomanip>


#include "weather_handler.h"
#include "query_utils.h"
#include "weather_api.h"
#include "weather_parser.h"



namespace weather_handler {

namespace {

bool IsLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DaysInMonth(int year, int month) {
    switch(month) {
        case 1:
            return 31;
        case 2:
            return IsLeapYear(year) ? 29 : 28;
        case 3:
            return 31;
        case 4:
            return 30;
        case 5:
            return 31;
        case 6:
            return 30;
        case 7:
            return 31;
        case 8:
            return 31;
        case 9:
            return 30;
        case 10:
            return 31;
        case 11:
            return 30;
        case 12:
            return 31;
        default:
            throw std::invalid_argument("Invalid month: > 12 or < 1\n");
    }
}

void ValidateDate(const std::string& date) {
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
        throw std::invalid_argument("Invalid date format. Expected format: YYYY-MM-DD\n");
    }

    for (size_t i = 0; i < date.length(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) {
            throw std::invalid_argument("Invalid date format. Expected format: YYYY-MM-DD\n");
        }
    }

    int year = std::stoi(date.substr(0, 4));

    if (year == 0) {
        throw std::invalid_argument("Invalid year: 0000\n");
    }
    
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    if (DaysInMonth(year, month) < day || day < 1) {
        throw std::invalid_argument("Invalid day: More days than there should be in a month or 0\n");
    }
}

void ValidateWeatherParams(const std::unordered_map<std::string, std::string>& params) {
    auto it = params.find("city");
    if (it == params.end() || it->second.empty()) {
        throw std::invalid_argument("City is missing or empty in the parameters\n");
    }

    it = params.find("date");
    if (it == params.end() || it->second.empty()) {
        throw std::invalid_argument("The date is missing or empty in the parameters\n");
    }

    ValidateDate(it->second);
    
}

} // namespace

simple_http_server::HttpResponse HandleWeatherRequest(const simple_http_server::HttpRequest& request) {
    auto query = request.uri().query();
    auto query_params = query_utils::ParseQueryParameters(query);
    ValidateWeatherParams(query_params);

    std::string weather_json = weather_api::FetchWeatherJson(query_params.at("city"), query_params.at("date"));

    auto weather_data = weather_parser::ParseWeatherJson(weather_json);

        std::ostringstream output;

    output << "Город: " << weather_data.city << '\n'
           << "Дата: " << weather_data.date << '\n'
           << "Текущая температура: " << std::fixed << std::setprecision(1) << weather_data.current_temp_c << " °C\n"
           << "Погодные условия: " << weather_data.condition << '\n'
           << "Минимальная температура за день: " << weather_data.min_temp_c << " °C\n"
           << "Максимальная температура за день: " << weather_data.max_temp_c << " °C\n";

    simple_http_server::HttpResponse response(simple_http_server::HttpStatusCode::Ok);
    response.SetHeader("Content-Type", "text/plain; charset=utf-8");
    response.SetContent(output.str());

    return response;
}

}  // namespace weather_handler