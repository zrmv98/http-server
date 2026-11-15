// Define object that can be used to represent a Unique Resource Identifier
// which is useful for parsing and request handling

#ifndef URI_H_
#define URI_H_

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <utility>

namespace simple_http_server {

// A Uri object will contain a valid scheme (for example: HTTP), host,
// port, and the actual URI path
class Uri {
 public:
  Uri() = default;
  explicit Uri(const std::string& path) {
    SetPath(path);
  }
  ~Uri() = default;

  inline bool operator<(const Uri& other) const { return path_ < other.path_; }
  inline bool operator==(const Uri& other) const {
    return path_ == other.path_;
  }

  void SetPath(const std::string& path) {
    auto pos = path.find('?');

    if (pos != std::string::npos) {
      path_ = path.substr(0, pos);
      query_ = path.substr(pos + 1);
    } else {
      path_ = path;
      query_.clear();
    }

    SetPathToLowercase();
  }

  std::string scheme() const { return scheme_; }
  std::string host() const { return host_; }
  std::uint16_t port() const { return port_; }
  std::string path() const { return path_; }
  std::string query() const { return query_; }

 private:
  // Parses the request path and query string.
  // Scheme, host and port are not parsed.
  std::string path_;
  std::string query_;
  std::string scheme_;
  std::string host_;
  std::uint16_t port_;

  void SetPathToLowercase() {
    std::transform(path_.begin(), path_.end(), path_.begin(),
                   [](char c) { return tolower(c); });
  }
};

}  // namespace simple_http_server

#endif  // URI_H_
