SHELL=/bin/bash

CXX := g++
CXXFLAGS := -std=c++11 -O3 -pthread

TARGET_EXEC := thread_pool 

BUILD_DIR := ./build
SRC_DIR := .

SRCS := $(shell find $(SRC_DIR) -name '*.cpp' -or -name '*.cc')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)