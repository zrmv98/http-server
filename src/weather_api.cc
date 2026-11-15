#include <curl/curl.h>
#include <stdexcept>
#include <string>
#include <memory>
#include <cstdlib>

#include "weather_api.h"

namespace weather_api {

CurlGlobal::CurlGlobal() {
    CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to initialize libcurl: ") + curl_easy_strerror(result) + '\n');
    }
}

CurlGlobal::~CurlGlobal() {
    curl_global_cleanup();
}

size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;

    auto* response = static_cast<std::string*>(userdata);

    if (!response) {
        return 0; // Return 0 to indicate an error
    }
    
    try {
        response->append(ptr, total_size);
    } catch (...) {
        return 0;
    }

    return total_size;
}

std::string FetchWeatherJson(const std::string& city, const std::string& date) {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), &curl_easy_cleanup);

    if (!curl) {
        throw std::runtime_error("Failed to initialize curl_easy\n");
    }

    std::string response;

    auto result = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to set write callback: ") + curl_easy_strerror(result) +  '\n');
    }

    result = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to set write data: ") + curl_easy_strerror(result) + '\n');
    }

    const char* api_key = std::getenv("WEATHER_API_KEY");

    if (api_key == nullptr || api_key[0] == '\0') {
        throw std::runtime_error("WEATHER_API_KEY is not set\n");
    }

    std::unique_ptr<char, decltype(&curl_free)> encoded_city(curl_easy_escape(curl.get(), city.c_str(), 0), &curl_free);

    if (!encoded_city) {
        throw std::runtime_error("Failed to URL-encode city\n");
    }

    std::string url = "https://api.weatherapi.com/v1/forecast.json";

    url += "?key=";
    url += api_key;

    url += "&q=";
    url += encoded_city.get();

    url += "&dt=";
    url += date;

    url += "&days=1";

    result = curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to set URL: ") + curl_easy_strerror(result) + '\n');
    }

    result = curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to set NOSIGNAL: ") + curl_easy_strerror(result) + '\n');
    }

    result = curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to set CONNECTTIMEOUT: ") + curl_easy_strerror(result) + '\n');
    }

    result = curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 15L);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to set TIMEOUT: ") + curl_easy_strerror(result) + '\n');
    }

    result = curl_easy_perform(curl.get());

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to perform weather request: ") + curl_easy_strerror(result) + '\n');
    }

    long http_status = 0;

    result = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_status);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Failed to get HTTP status code: ") + curl_easy_strerror(result) + '\n');
    }

    if (http_status != 200) {
        throw std::runtime_error("Weather API request failed with HTTP status code: " + std::to_string(http_status) + '\n');
    }

    return response;
}

} // weather_api