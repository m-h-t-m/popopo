CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# 所有可执行文件
TARGETS = list listtemplate

# 默认目标
all: $(TARGETS)

# 通用规则
%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

.PHONY: all clean
