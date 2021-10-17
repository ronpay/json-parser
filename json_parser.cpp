#include "json_parser.hpp"

using namespace json;

// class json_node
// class json_node

// class json_char_reader
json_char_reader::json_char_reader(std::string& str) : in(str) {}
char json_char_reader::get()
{
    return in.get();
}
char json_char_reader::peek()
{
    return in.peek();
}
std::string json_char_reader::next(int cnt)
{
    std::string s;
    while (cnt--) {
        s.push_back(in.get());
    }
    return s;
}
bool json_char_reader::has_more()
{
    return !in.eof();
}
// class json_char_reader

// class json_token_reader
json_token_reader::json_token_reader(std::string& str) : char_reader(str) {}
token_type json_token_reader::next_token()
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
        case '-':
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
            throw std::runtime_error(fmt::format("Unexpected json char : {}", c));
            break;
    }
    return token;
}
bool json_token_reader::read_boolean()
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
double json_token_reader::read_number()
{
    double      ret;
    bool        neg = false;
    std::string s;
    if (char_reader.peek() == '-') {
        char_reader.get();
        neg = true;
    }
    while (isdigit(char_reader.peek())) {
        s.push_back(char_reader.get());
    }
    if (s.empty()) {
        throw std::runtime_error("Invalid number");
    }
    ret = std::stod(s) * (neg ? -1 : 1);
    return ret;
}
std::string json_token_reader::read_string()
{
    std::string ret;
    char_reader.get();  // skip '"'
    while (char_reader.peek() != '"') {
        ret.push_back(char_reader.get());
    }
    char_reader.get();  // skip '"'
    return ret;
}
void json_token_reader::pass_char()
{
    char_reader.get();
}
void json_token_reader::read_null()
{
    char_reader.next(4);
}
// class json_token_reader

std::ostream& operator<<(std::ostream& os, json_value& jv)
{
    os << jv.to_string();
    return os;
}


auto json_value::get() const
{
    return json;
}
std::string json_value::get_string() const
{
    return std::get<std::string>(json);
}
double json_value::get_number() const
{
    return std::get<double>(json);
}
bool json_value::get_boolean() const
{
    return std::get<bool>(json);
}
std::vector<std::shared_ptr<json_node>>& json_value::get_array ()
{
    return std::get<std::vector<std::shared_ptr<json_node>>>(json);
}
std::unordered_map<std::string, std::shared_ptr<json_node>>& json_value::get_object ()
{
    return std::get<std::unordered_map<std::string, std::shared_ptr<json_node>>>(json);
}
template <class T> void json_value::set(T value)
{
    json = value;
}
// void set(double value){
//     json = value;
// }
void json_value::put_value(std::string key, json_value value)
{
    std::get<std::unordered_map<std::string, std::shared_ptr<json_node>>>(json)[key] = std::make_shared<json_value>(value);
}
void json_value::push_array(json_value value)
{
    // std::cout << "push array" << std::endl;
    // if (value.has_type<double>()) {
    //     std::cout << "push array number: " << value.get_number() << std::endl;
    // }
    std::get<std::vector<std::shared_ptr<json_node>>>(json).push_back(std::make_shared<json_value>(value));
}

template <class T> bool json_value::has_type()
{
    return std::holds_alternative<T>(json);
}

//* 访问json

json_value& json_value::operator[](std::string key)
{
    if (has_type<std::unordered_map<std::string, std::shared_ptr<json_node>>>()) {
        std::unordered_map<std::string, std::shared_ptr<json_node>>& object = std::get<std::unordered_map<std::string, std::shared_ptr<json_node>>>(json);
        if (object.count(key) == 0) {
            throw std::runtime_error("json access object error : key not found");
        }
        return *(std::static_pointer_cast<json_value>(object[key]));
    }
    throw std::runtime_error("json access object error.");
}
json_value& json_value::operator[](std::size_t index)
{
    if (has_type<std::vector<std::shared_ptr<json_node>>>()) {
        std::vector<std::shared_ptr<json_node>>& array = std::get<std::vector<std::shared_ptr<json_node>>>(json);
        if (index < 0 || index >= array.size()) {
            throw std::runtime_error("json access array error : index out of range");
        }
        return *std::static_pointer_cast<json_value>(std::get<std::vector<std::shared_ptr<json_node>>>(json)[index]);
    }
    throw std::runtime_error("json access arrary error.");
}

std::string json_value::to_string()
{
    // std::cout << "enter to string entry\n";
    std::string ret;
    if (has_type<std::string>()) {
        ret.push_back('"');
        ret.append(get_string());
        ret.push_back('"');
        return ret;
    }
    if (has_type<double>()) {
        // std::cout << "enter number to string\n";
        ret = std::to_string(get_number());
        return ret;
    }
    if (has_type<bool>()) {
        return get_boolean() ? "true" : "false";
    }
    if (has_type<std::vector<std::shared_ptr<json_node>>>()) {
        // std::cout << "enter array to string\n";
        ret.push_back('[');
        // std::cout << "enter array to string push [\n";
        for (auto v : get_array()) {
            // if (std::static_pointer_cast<json_value>(v)->has_type<double>()) {
            //     std::cout << "double member\n";
            // }
            // std::cout << "array member:\n" << std::static_pointer_cast<json_value>(v)->to_string() << std::endl;
            ret.append(std::static_pointer_cast<json_value>(v)->to_string());
            ret.push_back(',');
        }
        if (ret.back() == ',') {
            ret.pop_back();
        }
        // std::cout << "enter array to string push ]\n";
        ret.push_back(']');
        return ret;
    }
    if (has_type<std::unordered_map<std::string, std::shared_ptr<json_node>>>()) {
        ret.push_back('{');
        for (auto& v : get_object()) {
            ret.push_back('"');
            ret.append(v.first);
            ret.push_back('"');
            ret.push_back(':');
            ret.append(std::static_pointer_cast<json_value>(v.second)->to_string());
            ret.push_back(',');
        }
        if (ret.back() == ',') {
            ret.pop_back();
        }
        ret.push_back('}');
        return ret;
    }
    if (has_type<nullptr_t>()) {
        return "null";
    }

    throw std::runtime_error("json to string error.");
}

// class json {};

json_parser::json_parser(std::string& str) : token_reader(str) {}
json_value json_parser::parse()
{
    std::stack<json_value> json_stack;
    uint16_t               expect    = EXPECT_SINGLE_VALUE | EXPECT_BEGIN_ARRAY | EXPECT_BEGIN_OBJECT;
    // int                    token_cnt = 0;
    while (true) {
        token_type token = token_reader.next_token();
        // std::cout << fmt::format("token {} : {}\n", token_cnt++, token);
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
                    // std::cout << "json object key:" << s << std::endl;
                    json_stack.pop();
                    json_stack.top().put_value(s, json_value(token_reader.read_number()));
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
                    json_stack.top().push_array(json_value(token_reader.read_boolean()));
                    expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                    continue;
                }
                if (expect & EXPECT_OBJECT_VALUE) {
                    std::string s = json_stack.top().get_string();
                    json_stack.pop();
                    json_stack.top().put_value(s, json_value(token_reader.read_boolean()));
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
                    json_stack.top().push_array(json_value(token_reader.read_string()));
                    expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                    continue;
                }
                if (expect & EXPECT_OBJECT_VALUE) {
                    std::string s = json_stack.top().get_string();
                    json_stack.pop();
                    json_stack.top().put_value(s, json_value(token_reader.read_string()));
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
                token_reader.read_null();
                if (expect & EXPECT_SINGLE_VALUE) {
                    json_value json;
                    json.set(nullptr);
                    json_stack.push(json);
                    expect = EXPECT_END_DOCUMENT;
                    continue;
                }
                if (expect & EXPECT_ARRAY_VALUE) {
                    json_stack.top().push_array(json_value(nullptr));
                    expect = EXPECT_END_ARRAY | EXPECT_COMMA;
                    continue;
                }
                if (expect & EXPECT_OBJECT_VALUE) {
                    std::string s = json_stack.top().get_string();
                    json_stack.pop();
                    json_stack.top().put_value(s, json_value(nullptr));
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
                    expect = EXPECT_ARRAY_VALUE | EXPECT_BEGIN_OBJECT | EXPECT_BEGIN_ARRAY | EXPECT_END_ARRAY;
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
                    // std::cout << "Enter end object\n";
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
                    if (json_stack.top().has_type<std::vector<std::shared_ptr<json_node>>>()) {
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
                        // std::cout << "json parse success, return json." << std::endl;
                        return json;
                    }
                }
                throw std::runtime_error("Unexpected end of document.");
            }
        }
    }
};
