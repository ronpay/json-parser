#include <cctype>
#include <cstdio>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace json {

enum token_type : uint16_t {
    END_DOCUMENT = 1,
    BEGIN_OBJECT = 2,
    END_OBJECT   = 4,
    BEGIN_ARRAY  = 8,
    END_ARRAY    = 16,
    SEP_COLON    = 32,
    SEP_COMMA    = 64,
    STRING       = 128,
    NUMBER       = 256,
    BOOLEAN      = 512,
    NULL_VALUE   = 1024,
    BLANK        = 2048
};

enum expect_token_type : uint16_t {
    EXPECT_SINGLE_VALUE = 1,
    EXPECT_BEGIN_ARRAY  = 2,
    EXPECT_END_ARRAY    = 4,
    EXPECT_ARRAY_VALUE  = 8,

    EXPECT_BEGIN_OBJECT = 16,
    EXPECT_END_OBJECT   = 32,
    EXPECT_OBJECT_KEY   = 64,
    EXPECT_OBJECT_VALUE = 128,

    EXPECT_COLON = 256,
    EXPECT_COMMA = 512,

    EXPECT_END_DOCUMENT = 1024
};

class json_char_reader {
  public:
    json_char_reader(std::string& str) : in(str) {}
    char get()
    {
        return in.get();
    }
    char peek()
    {
        return in.peek();
    }
    std::string next(int cnt)
    {
        std::string s;
        while (cnt--) {
            s.push_back(in.get());
        }
        return s;
    }
    bool has_more()
    {
        return !in.eof();
    }

  private:
    std::ifstream in;
};  // class json_char_reader

class json_token_reader {
  public:
    json_token_reader(std::string& str) : char_reader(str) {}
    token_type next_token()
    {
        token_type token;
        char       c = char_reader.peek();
        if (c == -1) {
            token = END_DOCUMENT;
            return token;
        }
        switch (c) {
            case '{': token = BEGIN_OBJECT; break;
            case '}': token = END_OBJECT; break;
            case '[': token = BEGIN_ARRAY; break;
            case ']': token = END_ARRAY; break;
            case ':': token = SEP_COLON; break;
            case ',': token = SEP_COMMA; break;
            case '"': token = STRING; break;
            case 't':
            case 'f': token = BOOLEAN; break;
            case 'n': token = NULL_VALUE; break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': token = NUMBER; break;
            case ' ':
            case '\n':
            case '\r': token = BLANK; break;
            default:
                printf("%i\n", c);
                throw std::runtime_error(fmt::format("Unexpect json char : {}", c));
                break;
        }
        return token;
    }
    bool read_boolean()
    {
        bool ret;
        char c = char_reader.peek();
        switch (c) {
            case 't':
                ret = true;
                char_reader.next(4);
                break;
            case 'f':
                ret = false;
                char_reader.next(5);
                break;
        }
        return ret;
    }
    double read_number()
    {
        double      ret;
        std::string s;
        while (isdigit(char_reader.peek())) {
            s.push_back(char_reader.get());
        }
        ret = std::stod(s);
        return ret;
    }
    std::string read_string()
    {
        std::string ret;
        char_reader.get();  // skip '"'
        while (char_reader.peek() != '"') {
            ret.push_back(char_reader.get());
        }
        char_reader.get();  // skip '"'
        return ret;
    }
    void pass_char()
    {
        char_reader.get();
    }
    void pass_null()
    {
        char_reader.next(4);
    }

  private:
    json_char_reader char_reader;
};  // class json_token_reader

class json_node {
  public:
  private:
};  // class json_node

class json_value : public json_node {
  public:
    json_value() : json(nullptr) {}
    json_value(std::string s) : json(s) {}
    json_value(double d) : json(d) {}
    json_value(bool b) : json(b) {}
    json_value(std::nullptr_t) : json(nullptr) {}
    json_value(std::vector<std::shared_ptr<json_node>> v) : json(v) {}
    json_value(std::unordered_map<std::string, std::shared_ptr<json_node>> m) : json(m) {}

    auto get() const
    {
        return json;
    }
    std::string get_string() const
    {
        return std::get<std::string>(json);
    }
    double get_number() const
    {
        return std::get<double>(json);
    }
    bool get_boolean() const
    {
        return std::get<bool>(json);
    }
    std::vector<std::shared_ptr<json_node>>& get_array()
    {
        return std::get<std::vector<std::shared_ptr<json_node>>>(json);
    }
    std::unordered_map<std::string, std::shared_ptr<json_node>>& get_object()
    {
        return std::get<std::unordered_map<std::string, std::shared_ptr<json_node>>>(json);
    }
    template <class T> void set(T value)
    {
        json = value;
    }
    // void set(double value){
    //     json = value;
    // }
    void put_value(std::string key, json_value value)
    {
        std::get<std::unordered_map<std::string, std::shared_ptr<json_node>>>(json)[key] = std::make_shared<json_value>(value);
    }
    void push_array(json_value value)
    {   
        std::cout<<"push array"<<std::endl;
        if(value.has_type<double>()){
            std::cout<<"push array number: "<<value.get_number()<<std::endl;
        }
        std::get<std::vector<std::shared_ptr<json_node>>>(json).push_back(std::make_shared<json_value>(value));
    }

    template <class T> bool has_type()
    {
        return std::holds_alternative<T>(json);
    }

    //* 访问json

    auto operator[](std::string key)
    {
        if (has_type<std::unordered_map<std::string, std::shared_ptr<json_node>>>()) {
            return std::get<std::unordered_map<std::string, std::shared_ptr<json_node>>>(json)[key];
        }
        throw std::runtime_error("json access object error.");
    }
    auto operator[](int index)
    {
        if (has_type<std::vector<std::shared_ptr<json_node>>>()) {
            return std::get<std::vector<std::shared_ptr<json_node>>>(json)[index];
        }
        throw std::runtime_error("json access arrary error.");
    }

    std::string to_string()
    {
        std::cout << "enter to string entry\n";
        std::string ret;
        if (has_type<std::string>()) {
            ret.push_back('"');
            ret.append(get_string());
            ret.push_back('"');
            return ret;
        }
        if (has_type<double>()) {
            std::cout << "enter number to string\n";
            ret = std::to_string(get_number());
            return ret;
        }
        if (has_type<bool>()) {
            return get_boolean() ? "true" : "false";
        }
        if (has_type<std::vector<std::shared_ptr<json_node>>>()) {
            std::cout << "enter array to string\n";
            ret.push_back('[');
            std::cout << "enter array to string push [\n";
            for (auto v : get_array()) {
                if(std::static_pointer_cast<json_value>(v)->has_type<double>()){
                    std::cout << "double member\n";
                }
                    std::cout << "array member:\n" << std::static_pointer_cast<json_value>(v)->to_string() << std::endl;
                    ret.append(std::static_pointer_cast<json_value>(v)->to_string());
                    ret.push_back(',');
            }
            // ret.pop_back();
            std::cout << "enter array to string push ]\n";
            ret.push_back(']');
            return ret;
        }
        if (has_type<std::unordered_map<std::string, std::shared_ptr<json_node>>>()) {
            ret.push_back('{');
            for (auto& v : get_object()) {
                ret.append(v.first);
                ret.push_back(':');
                ret.append(std::static_pointer_cast<json_value>(v.second)->to_string());
                ret.push_back(',');
            }
            ret.pop_back();
            ret.push_back('}');
            return ret;
        }
        if (has_type<nullptr_t>()) {
            return "null";
        }

        throw std::runtime_error("json to string error.");
    }

  private:
    std::variant<std::string, double, std::unordered_map<std::string, std::shared_ptr<json_node>>, bool, nullptr_t, std::vector<std::shared_ptr<json_node>>>
        json;
};  // class json_value

// class json {};

class json_parser {
  public:
    json_parser(std::string& str) : token_reader(str) {}
    json_value parse()
    {
        std::stack<json_value> json_stack;
        uint16_t               expect = EXPECT_SINGLE_VALUE | EXPECT_BEGIN_ARRAY | EXPECT_BEGIN_OBJECT;
        int                    token_cnt = 0;
        while (true) {
            token_type token = token_reader.next_token();
            std::cout << fmt::format("toekn {} : {}\n", token_cnt++, token);
            switch (token) {
                case BLANK: {
                    token_reader.pass_char();
                    continue;
                }
                case NUMBER: {
                    if (expect & EXPECT_SINGLE_VALUE) {
                        json_value json(token_reader.read_number());
                        json_stack.push(json);
                        expect = EXPECT_END_DOCUMENT;
                        continue;
                    }
                    if (expect & EXPECT_ARRAY_VALUE) {
                        json_stack.top().push_array(json_value(token_reader.read_number()));
                        // std::cout<<"json array test:"<<json_stack.top().to_string()<<std::endl;
                        expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                        continue;
                    }
                    if (expect & EXPECT_OBJECT_VALUE) {
                        std::string s = json_stack.top().get_string();
                        json_stack.pop();
                        json_value json = json_stack.top();
                        json.put_value(s, token_reader.read_number());
                        expect = EXPECT_END_OBJECT | EXPECT_COMMA;
                        continue;
                    }
                    throw std::runtime_error("Unexpected number.");
                }
                case BOOLEAN: {
                    if (expect & EXPECT_SINGLE_VALUE) {
                        json_value json(token_reader.read_boolean());
                        json_stack.push(json);
                        expect = EXPECT_END_DOCUMENT;
                        continue;
                    }
                    if (expect & EXPECT_ARRAY_VALUE) {
                        json_stack.top().push_array(token_reader.read_boolean());
                        expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                        continue;
                    }
                    if (expect & EXPECT_OBJECT_VALUE) {
                        std::string s = json_stack.top().get_string();
                        json_stack.pop();
                        json_value json = json_stack.top();
                        json.put_value(s, token_reader.read_boolean());
                        expect = EXPECT_END_OBJECT | EXPECT_COMMA;
                        continue;
                    }
                    throw std::runtime_error("Unexpected boolean.");
                }
                case STRING: {
                    if (expect & EXPECT_SINGLE_VALUE) {
                        json_value json(token_reader.read_string());
                        json_stack.push(json);
                        expect = EXPECT_END_DOCUMENT;
                        continue;
                    }
                    if (expect & EXPECT_ARRAY_VALUE) {
                        json_stack.top().push_array(token_reader.read_string());
                        expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                        continue;
                    }
                    if (expect & EXPECT_OBJECT_VALUE) {
                        std::string s = json_stack.top().get_string();
                        json_stack.pop();
                        json_value json = json_stack.top();
                        json.put_value(s, token_reader.read_string());
                        expect = EXPECT_END_OBJECT | EXPECT_COMMA;
                        continue;
                    }
                    if (expect & EXPECT_OBJECT_KEY) {
                        json_stack.push(json_value(token_reader.read_string()));
                        expect = EXPECT_COLON;
                        continue;
                    }
                    throw std::runtime_error("Unexpected string.");
                }
                case NULL_VALUE: {
                    token_reader.pass_null();
                    if (expect & EXPECT_SINGLE_VALUE) {
                        json_value json;
                        json.set(nullptr);
                        json_stack.push(json);
                        expect = EXPECT_END_DOCUMENT;
                        continue;
                    }
                    if (expect & EXPECT_ARRAY_VALUE) {
                        json_stack.top().push_array(nullptr);
                        expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                        continue;
                    }
                    if (expect & EXPECT_OBJECT_VALUE) {
                        std::string s = json_stack.top().get_string();
                        json_stack.pop();
                        json_value json = json_stack.top();
                        json.put_value(s, nullptr);
                        expect = EXPECT_END_OBJECT | EXPECT_COMMA;
                        continue;
                    }
                    throw std::runtime_error("Unexpected null.");
                }
                case BEGIN_ARRAY: {
                    token_reader.pass_char();
                    if (expect & EXPECT_BEGIN_ARRAY) {
                        std::vector<std::shared_ptr<json_node>> v;
                        json_stack.push(json_value(v));
                        expect = EXPECT_ARRAY_VALUE | EXPECT_BEGIN_OBJECT|EXPECT_BEGIN_ARRAY|EXPECT_END_ARRAY;
                        continue;
                    }
                    throw std::runtime_error("Unexpected begin of array : [.");
                }
                case BEGIN_OBJECT: {
                    token_reader.pass_char();
                    if (expect & EXPECT_BEGIN_OBJECT) {
                        std::unordered_map<std::string, std::shared_ptr<json_node>> m;
                        json_stack.push(json_value(m));
                        expect = EXPECT_OBJECT_KEY | EXPECT_BEGIN_OBJECT | EXPECT_END_OBJECT;
                        continue;
                    }
                    throw std::runtime_error("Unexpected begin of array : {.");
                }
                case END_ARRAY: {
                    token_reader.pass_char();
                    if (expect & EXPECT_END_ARRAY) {
                        json_value array = json_stack.top();
                        json_stack.pop();
                        if (json_stack.empty()) {
                            json_stack.push(array);
                            expect = EXPECT_END_DOCUMENT;
                            continue;
                        }
                        if (json_stack.top().has_type<std::string>()) {
                            std::string s = json_stack.top().get_string();
                            json_stack.pop();
                            json_stack.top().put_value(s, array);
                            expect = EXPECT_COMMA | EXPECT_END_OBJECT;
                            continue;
                        }
                        if (json_stack.top().has_type<std::vector<std::shared_ptr<json_node>>>()) {
                            json_stack.top().push_array(array);
                            expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                            continue;
                        }
                    }
                    throw std::runtime_error("Unexpected end of array : ].");
                }
                case END_OBJECT: {
                    token_reader.pass_char();
                    if (expect & EXPECT_END_OBJECT) {
                        json_value object = json_stack.top();
                        json_stack.pop();
                        if (json_stack.empty()) {
                            json_stack.push(object);
                            expect = EXPECT_END_DOCUMENT;
                            continue;
                        }
                        if (json_stack.top().has_type<std::string>()) {
                            std::string s = json_stack.top().get_string();
                            json_stack.pop();
                            json_stack.top().put_value(s, object);
                            expect = EXPECT_COMMA | EXPECT_END_OBJECT;
                            continue;
                        }
                        if (json_stack.top().has_type<std::unordered_map<std::string, std::shared_ptr<json_node>>>()) {
                            json_stack.top().push_array(object);
                            expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                            continue;
                        }
                    }
                    throw std::runtime_error("Unexpected end of object : }.");
                }
                //* :
                case SEP_COLON: {
                    token_reader.pass_char();
                    if (expect & EXPECT_COLON) {
                        expect = EXPECT_OBJECT_VALUE | EXPECT_BEGIN_ARRAY | EXPECT_BEGIN_OBJECT;
                        continue;
                    }
                    throw std::runtime_error("Unexpected colon.");
                }
                //* ,
                case SEP_COMMA: {
                    token_reader.pass_char();
                    if (expect & EXPECT_COMMA) {
                        if (expect & EXPECT_END_OBJECT) {
                            expect = EXPECT_OBJECT_KEY;
                            continue;
                        }
                        if (expect & EXPECT_END_ARRAY) {
                            expect = EXPECT_ARRAY_VALUE | EXPECT_BEGIN_ARRAY | EXPECT_BEGIN_OBJECT;
                            continue;
                        }
                    }
                    throw std::runtime_error("Unexpected comma.");
                }
                case END_DOCUMENT: {
                    if (expect & EXPECT_END_DOCUMENT) {
                        json_value json = json_stack.top();
                        json_stack.pop();
                        if (json_stack.empty()) {
                            std::cout << "json parse success, return json." << std::endl;
                            return json;
                        }
                    }
                    throw std::runtime_error("Unexpected end of document.");
                }
            }
        }
    };

  private:
    json_token_reader token_reader;
};  // class json_parser
};  // namespace json

int main()
{
    std::string       file = "test.json";
    json::json_parser parser(file);
    json::json_value  json = parser.parse();
    std::cout << json.to_string() << std::endl;
    return 0;
}
