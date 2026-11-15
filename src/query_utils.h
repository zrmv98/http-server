#pragma once

#include <string>
#include <unordered_map>

namespace query_utils {

std::string UrlDecode(const std::string& input);

std::unordered_map<std::string, std::string> ParseQueryParameters(const std::string& query);

} // namespace query_utils