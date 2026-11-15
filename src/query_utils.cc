#include <stdexcept>

#include "query_utils.h"

namespace query_utils {

namespace {

int HexToInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    throw std::invalid_argument("Invalid hexadecimal digit\n");
}

}  // namespace

std::string UrlDecode(const std::string& input) {
    std::string result;

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '+') {
            result += ' ';
        } else if (input[i] != '%') {
            result += input[i];
        } else {
            if (input.size() - i >= 3) {
                int a = HexToInt(input[i + 1]);
                int b = HexToInt(input[i + 2]);

                result += static_cast<char>(a * 16 + b);

                i += 2;
            } else {
                throw std::invalid_argument("Invalid percent sequence");
            }
        }
    }

    return result;
}

std::unordered_map<std::string, std::string> ParseQueryParameters(const std::string& query) {
    std::unordered_map<std::string, std::string> query_params;

    size_t param_start = 0;

    while (param_start < query.size()) {
        auto amp_pos = query.find('&', param_start);

        auto param_end = (amp_pos == std::string::npos) ? query.size() : amp_pos;

        if (param_start != param_end) {
            std::string param = query.substr(param_start, param_end - param_start);

            auto equals_pos = param.find('=');

            if (equals_pos == std::string::npos) {
                throw std::invalid_argument("Invalid query parameter\n");
            }

            std::string key = UrlDecode(param.substr(0, equals_pos));
            std::string value = UrlDecode(param.substr(equals_pos + 1));

            if (key.empty()) {
                throw std::invalid_argument("Empty query key\n");
            }

            auto result = query_params.emplace(key, value);

            if (!result.second) {
                throw std::invalid_argument("Duplicate query parameter\n");
            }
        }

        if (amp_pos == std::string::npos) {
            break;
        }

        param_start = amp_pos + 1;
    }

    return query_params;
}

}  // namespace query_utils