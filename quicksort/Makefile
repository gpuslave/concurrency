SHELL=/bin/bash

CXX := g++
CXXFLAGS := -std=c++17 -O3 -pthread

TARGET_EXEC := asyncqsort

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

# g++ -std=c++17 -O3 quicksort/ParallelQuickSort.cpp -pthread -o run