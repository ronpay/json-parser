#pragma once

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

class json_value;

enum token_type : uint16_t {
    END_DOCUMENT = 1,
    BEGIN_OBJECT = 2,
    END_OBJECT   = 4,
    BEGIN_ARRAY  = 8,
    END_ARRAY    = 16,
    SEP_COLON    = 32,  // :
    SEP_COMMA    = 64,  // ,
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

class json_node {
  public:
  private:
};  // class json_node

class json_char_reader {
  public:
    json_char_reader(std::string& str);
    char get();
    char        peek();
    std::string next(int cnt);
    bool        has_more();

  private:
    std::ifstream in;
};  // class json_char_reader

class json_token_reader {
  public:
    json_token_reader(std::string& str);
    token_type next_token();
    bool       read_boolean();
    double     read_number();

    std::string read_string();

    void pass_char();
    void pass_null();

  private:
    json_char_reader char_reader;
};  // class json_token_reader

//std::ostream& operator<<(std::ostream& os, json_value& jv);

class json_value : public json_node {
  public:
    explicit json_value() : json(nullptr) {}
    explicit json_value(std::string s): json(s) {}
    explicit json_value(double d): json(d) {}
    explicit json_value(bool b): json(b) {}
    explicit json_value(std::nullptr_t): json(nullptr) {}
    explicit json_value(std::vector<std::shared_ptr<json_node>> v): json(v) {}
    explicit json_value(std::unordered_map<std::string, std::shared_ptr<json_node>> m): json(m) {}

    auto        get() const;
    std::string get_string() const;
    double      get_number() const;
    bool                                     get_boolean() const;
    std::vector<std::shared_ptr<json_node>>&                     get_array();
    std::unordered_map<std::string, std::shared_ptr<json_node>>& get_object();
    template <class T> void                                      set(T value);
    // void set(double value){
    //     json = value;
    // }
    void put_value(std::string key, json_value value);
    void push_array(json_value value);

    template <class T> bool has_type();

    //* 访问json

    json_value& operator[](std::string key);
    json_value& operator[](std::size_t index);

    std::string to_string();

    // friend std::ostream& operator<<(std::ostream& os, json_value& jv);

  private:
    std::variant<std::string, double, std::unordered_map<std::string, std::shared_ptr<json_node>>, bool, nullptr_t, std::vector<std::shared_ptr<json_node>>>
        json;
};  // class json_value

class json_parser {
  public:
    json_parser(std::string& str);
    json_value parse();

  private:
    json_token_reader token_reader;
};  // class json_parser


}  // namespace json