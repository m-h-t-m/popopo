# 简单的 Makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

TARGET = hello
SOURCE = hello.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TARGET)

.PHONY: clean
