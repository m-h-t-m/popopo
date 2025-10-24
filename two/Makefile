# 简单的 Makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

TARGET = string
SOURCE = string.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TARGET)

.PHONY: clean
