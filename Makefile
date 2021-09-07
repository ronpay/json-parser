CXX = g++
CXXFLAGS = -g -m64 -Wall -std=c++17 -lfmt
TARGET = json_parser

$(TARGET): $(TARGET).cpp
	$(CXX) $(CXXFLAGS) -o $@ $<