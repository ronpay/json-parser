# json_parser

A tiny json parser, just for learning.

# Usage

```cpp
    json::json_parser parser("file.json");
    json::json_value  js = parser.parse();
    std::cout << "id:" << js["id"].to_string() << std::endl;
    std::cout << “num:" << js["arguments"]["game"][1].to_string() << std::endl;
```
