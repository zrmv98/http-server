#pragma once

#include <string>

namespace weather_api {

class CurlGlobal {
public:
    CurlGlobal();
    ~CurlGlobal();

    CurlGlobal(const CurlGlobal&) = delete;
    CurlGlobal& operator=(const CurlGlobal&) = delete;
};

std::string FetchWeatherJson(const std::string& city, const std::string& date);

} // weather_api