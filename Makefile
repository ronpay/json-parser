CXX = g++
CXXFLAGS = -g -m64 -Wall -static-libgcc -std=c++17
TARGET = json_parser

$(TARGET): $(TARGET).cpp
	$(CXX) $(CXXFLAGS) -o $@ $<