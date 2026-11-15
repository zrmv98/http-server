#include <utility>
#include <cctype>
#include <string_view>

#include "json.h"

using namespace std;

namespace json {

const Node::Value& Node::GetValue() const {
    return *this;
}

Node::Value& Node::GetValue() {
    return *this;
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(GetValue());
}

bool Node::IsDouble() const {
    return IsInt() || std::holds_alternative<double>(GetValue());
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(GetValue());
}

bool Node::IsBool() const {
     return std::holds_alternative<bool>(GetValue());
}

bool Node::IsString() const {
     return std::holds_alternative<std::string>(GetValue());
}

bool Node::IsNull() const {
     return std::holds_alternative<std::nullptr_t>(GetValue());
}

bool Node::IsArray() const {
     return std::holds_alternative<Array>(GetValue());
}

bool Node::IsMap() const {
     return std::holds_alternative<Dict>(GetValue());
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Node does not contain int");
    }

    return std::get<int>(GetValue());
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Node does not contain bool");
    }

    return std::get<bool>(GetValue());
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(GetValue());
    }

    if (IsInt()) {
        return static_cast<double>(std::get<int>(GetValue()));
    }

    throw std::logic_error("Node does not contain a double");
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Node does not contain string");
    }

    return std::get<std::string>(GetValue());
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Node does not contain Array");
    }

    return std::get<Array>(GetValue());
}

Array& Node::AsArray(){
    if (!IsArray()) {
        throw std::logic_error("Node does not contain Array");
    }

    return std::get<Array>(GetValue());
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Node does not contain Dict");
    }

    return std::get<Dict>(GetValue());
}

Dict& Node::AsMap() {
    if (!IsMap()) {
        throw std::logic_error("Node does not contain Dict");
    }

    return std::get<Dict>(GetValue());
}

bool Node::operator==(const Node& other) const{
    return GetValue() == other.GetValue();
}

bool Node::operator!=(const Node& other) const{
    return !(*this == other);
}

Document::Document(Node root) 
    : root_(std::move(root)) {

}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

namespace {

Node LoadNode(std::istream& input);
Node LoadArray(std::istream& input);
Node LoadDict(std::istream& input);
Node LoadString(std::istream& input);
Node LoadNumber(std::istream& input);
Node LoadBool(std::istream& input);
Node LoadNull(std::istream& input);

bool IsDelimiter(int ch) {
    return ch == std::char_traits<char>::eof() || ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'
        || ch == ',' || ch == ']' || ch == '}';
}

Node LoadNode(std::istream& input) {
    input >> std::ws;

    const auto ch = input.peek();

    if (ch == std::char_traits<char>::eof()) {
        throw ParsingError("Unexpected end of JSON");
    }

    if (ch == '[') {
        return LoadArray(input);
    } 
    
    if (ch == '{') {
        return LoadDict(input);
    } 
    
    if (ch == '"') {
        return LoadString(input);
    }

     if (ch == 't' || ch == 'f') {
        return LoadBool(input);
    }
    
    if (ch == 'n') {
        return LoadNull(input);
    }

    if (ch == '-' || std::isdigit(ch)) {
        return LoadNumber(input);
    }

    throw ParsingError("Invalid JSON value");
}

Node LoadNull(std::istream& input) {
    std::string_view expected_null = "null";

    for (char expected_ch : expected_null) {
        char actual;

        if (!input.get(actual) || actual != expected_ch) {
            throw ParsingError("Invalid JSON null value");
        }
    }

    const auto ch = input.peek();
    
    if (!IsDelimiter(ch)) {
        throw ParsingError("Invalid JSON null value");
    }

    return Node(nullptr);
}

Node LoadBool(std::istream& input) {
    const auto first_char = input.peek();

    std::string_view expected = (first_char == 't' ? "true" : "false");

    for (char expected_ch : expected) {
        char actual;

        if (!input.get(actual) || actual != expected_ch) {
            throw ParsingError("Invalid JSON boolean value");
        }
    }

    const auto ch = input.peek();
    
    if (!IsDelimiter(ch)) {
        throw ParsingError("Invalid JSON boolean value");
    }

    return Node(first_char == 't');
}

Node LoadString(std::istream& input) {
    input.get();

    std::string result;

    char ch;

    bool closed = false;

    while (input.get(ch)) {
        if (static_cast<unsigned char>(ch) < 0x20) {
            throw ParsingError("Invalid JSON string value");
        }

        if (ch == '"') {
            closed = true;
            break;
        }

        if (ch == '\\') {
            char escaped;

            if (!input.get(escaped)) {
                throw ParsingError("Invalid JSON string value");
            }

            switch (escaped) {
                case '"':
                    result += '"';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 't':
                    result += '\t';
                    break;
                default:
                    throw ParsingError("Invalid JSON string value");
            }
        } else {
            result += ch;
        }
    }

    if (!closed) {
        throw ParsingError("Invalid JSON string value");
    }

    return Node(std::move(result));
}

Node LoadNumber(std::istream& input) {
    std::string number_str;

    while (true) {
        const auto ch = input.peek();

        if (std::isdigit(ch) || ch == '-' || ch == '+' || ch == '.' || ch == 'e' || ch == 'E') {
            number_str += static_cast<char>(input.get());
        } else {
           break;
        }
    }

    if (number_str.empty() || !IsDelimiter(input.peek())) {
        throw ParsingError("Invalid JSON number value");
    }

    std::size_t pos = 0;
    if (number_str[pos] == '-') {
        ++pos;
    }

    if (pos == number_str.size() || !std::isdigit(number_str[pos])) {
        throw ParsingError("Invalid JSON number value");
    }

    if (number_str[pos] == '0') {
        ++pos;

        if (pos < number_str.size() && std::isdigit(number_str[pos])) {
            throw ParsingError("Invalid JSON number value");
        }
    } else {
        while (pos < number_str.size() && std::isdigit(number_str[pos])) {
            ++pos;
        }
    }

    if (pos < number_str.size() && number_str[pos] == '.') {
        ++pos;

        if (pos == number_str.size() || !std::isdigit(number_str[pos])) {
            throw ParsingError("Invalid JSON number value");
        }

        while (pos < number_str.size() && std::isdigit(number_str[pos])) {
            ++pos;
        }
    }

    if (pos < number_str.size() && (number_str[pos] == 'e' || number_str[pos] == 'E')) {
        ++pos;

        if (pos < number_str.size() && (number_str[pos] == '+' || number_str[pos] == '-')) {
            ++pos;
        }

        if (pos == number_str.size() || !std::isdigit(number_str[pos])) {
            throw ParsingError("Invalid JSON number value");
        }

        while (pos < number_str.size() && std::isdigit(number_str[pos])) {
            ++pos;
        }
    }

    if (pos != number_str.size()) {
        throw ParsingError("Invalid JSON number value");
    }

    const bool is_double = number_str.find_first_of(".eE") != std::string::npos;

    if (is_double) {
        try {
            return Node(std::stod(number_str));
        } catch (const std::invalid_argument&) {
            throw ParsingError("Invalid JSON number value");
        } catch (const std::out_of_range&) {
            throw ParsingError("Invalid JSON number value");
        }
    } else {
        try {
            return Node(std::stoi(number_str));
        } catch (const std::invalid_argument&) {
            throw ParsingError("Invalid JSON number value");
        } catch (const std::out_of_range&) {
            try {
                return Node(std::stod(number_str));
            } catch (const std::invalid_argument&) {
                throw ParsingError("Invalid JSON number value");
            } catch (const std::out_of_range&) {
                throw ParsingError("Invalid JSON number value");
            }
        }
    }
}

Node LoadArray(std::istream& input) {
    input.get();

    input >> std::ws;

    Array result;

    if (input.peek() == ']') {
        input.get();
        return Node(std::move(result));
    }

    while (true) {
        result.push_back(LoadNode(input));

        input >> std::ws;

        const auto ch = input.peek();

        if (ch == ']') {
            input.get();
            break;
        }

        if (ch != ',') {
            throw ParsingError("Invalid JSON array value");
        }

        input.get();
    }

    return Node(std::move(result));

}

Node LoadDict(std::istream& input) {
    input.get();

    input >> std::ws;

    Dict result;

    if (input.peek() == '}') {
        input.get();
        return Node(std::move(result));
    }

    while (true) {
        input >> std::ws;

        if (input.peek() != '"') {
            throw ParsingError("Invalid JSON dict key");
        }

        Node key_node = LoadString(input);
        std::string key = key_node.AsString();

        input >> std::ws;

        if (input.get() != ':') {
            throw ParsingError("Invalid JSON dict value");
        }

        result.emplace(std::move(key), LoadNode(input));

        input >> std::ws;

        if (input.peek() == '}') {
            input.get();
            break;
        }

        if (input.get() != ',') {
            throw ParsingError("Invalid JSON dict value");
        }
    }

    return Node(std::move(result));

}




template <typename Value>
void PrintValue(const Value& value, std::ostream& output);

void PrintValue(std::nullptr_t, std::ostream& output);
void PrintValue(const bool b, std::ostream& output);
void PrintValue(const std::string& value, std::ostream& output);
void PrintValue(const Array& value, std::ostream& output);
void PrintValue(const Dict& value, std::ostream& output);

void PrintNode(const Node& node, std::ostream& output);

void PrintNode(const Node& node, std::ostream& output) {
    std::visit(
        [&output](const auto& value) {
            PrintValue(value, output);
        },
        node.GetValue());
}

template <typename Value>
void PrintValue(const Value& value, std::ostream& output) {
    output << value;
}

void PrintValue(std::nullptr_t, std::ostream& output) {
    output << "null";
}

void PrintValue(const bool value, std::ostream& output) {
    output << (value ? "true" : "false");
}

void PrintValue(const std::string& value, std::ostream& output) {
    output << '"';

    for (const auto& ch : value) {
        switch (ch) {
            case '"':
                output << "\\\"";
                break;
            case '\\':
                output << "\\\\";
                break;
            case '\n':
                output << "\\n";
                break;
            case '\r': 
                output << "\\r";
                break;
            case '\t':
                output << "\\t";
                break;
            default:
                output << ch;
                break;        
        } 
    }

    output << '"';
}

void PrintValue(const Array& value, std::ostream& output) {
    output << "[";

    bool first = true;

    for (const auto& v : value) {
        if (!first) {
            output << ',';
        }

        PrintNode(v, output);

        first = false;
    }

    output << "]";
    
}

void PrintValue(const Dict& value, std::ostream& output) {
    output << "{";

    bool first = true;

    for (const auto& [key, val] : value) {
        if (!first) {
            output << ',';
        }

        PrintValue(key, output);

        output << ':';

        PrintNode(val, output);

        first = false;
    }

    output << "}";
}


} //namespace

Document Load(std::istream& input) {
    Node root = LoadNode(input);

    input >> std::ws;

    if (input.peek() != std::char_traits<char>::eof()) {
        throw ParsingError("Unexpected data after JSON value");
    }
    
    return Document{root};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json