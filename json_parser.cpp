#include <string>
#include <iostream>
#include <fmt/format.h>
#include <fstream>
#include <stack>
#include <any>
#include <vector>
#include <variant>
#include <utility>
#include <map>
#include <unordered_map>


namespace json{
    

enum token_type{
    END_DOCUMENT,
    BEGIN_OBJECT,
    END_OBJECT,
    BEGIN_ARRAY,
    END_ARRAY,
    SEP_COLON,
    SEP_COMMA,
    STRING,
    NUMBER,
    BOOLEAN,
    NULL_VALUE
};

class json_char_reader{
public:
    json_char_reader(std::string& str):in(str){
         
    }
    char get(){
        return in.get();
    }
    char peek(){
        return in.peek();
    }
    std::string next(int cnt){
        std::string s;
        while(cnt--){
            s.push_back(in.get());
        }
    }
    bool has_more(){
        return !in.eof();
    }
private:
    std::ifstream in;
};

class json_token_reader{
public:
    json_token_reader(std::string& str):char_reader(str){

    }
    token_type next_token(){
        token_type token;
        char c = char_reader.peek();
        switch(c){
            case '{':
                token=BEGIN_OBJECT;
                break;
            case '}':
                token=END_OBJECT;
                break;
            case '[':
                token=BEGIN_ARRAY;
                break;
            case ']':
                token=END_ARRAY;
                break;
            case ':':
                token=SEP_COLON;
                break;
            case ',':
                token=SEP_COMMA;
                break;
            case '"':
                token=STRING;
                break;
            case 't':
            case 'f':
                token=BOOLEAN;
                break;
            case 'n':
                token=NULL_VALUE;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                token=NUMBER;
                break;
            
        }
        return token;
    }
    bool read_boolean(){
        bool ret;
        char c = char_reader.peek();
        switch (c){
            case 't':
                ret=true;
                break;
            case 'f':
                ret=false;
                break;
        }
        char_reader.next(4);
        return ret;
    }
    double read_number(){
        double ret;
        std::string s;
        while(next_token()==NUMBER){
            s.push_back(char_reader.get());
        }
        ret = std::stod(s);
        return ret;
    }
    std::string read_string(){
        std::string ret;
        while(next_token()!=STRING){
            ret.push_back(char_reader.get());
        }
        return ret;
    }
    private:
        json_char_reader char_reader;

};

class json_node {
public:
private:
};

class json_value : public json_node {
public:
    auto get() const{
        return json;
    }
    std::string get_string() const{
        return std::get<std::string>(json);
    }
    double get_number() const{
        return std::get<double>(json);
    }
    bool get_boolean() const{
        return std::get<bool>(json);
    }
    std::vector<json_value> get_array() const{
        return std::get<std::vector<json_value>>(json);
    }
    std::unordered_map<std::string,json_value> get_object() const{
        return std::get<std::unordered_map<std::string, json_node>>(json);
    }
    template<class T> void set(T& value){
        json = value;
    }

private:
    std::variant<std::string, double, std::unordered_map<std::string, json_node>, bool, nullptr_t,std::vector<json_node>> json;
};

class json {};

class json_parse {
public:
    json_parse(std::string& str):token_reader(str){}
    json parse(){
        std::stack<std::any> stack;
        
    }
    std::string stringify(){
        
}
private:
    json_token_reader token_reader;
};

};
