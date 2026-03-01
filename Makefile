# Simple Makefile for miniCC project

CXX := g++
CXXFLAGS := -std=c++17 -Wall -I./src -Ivendor

SRCDIR := src
SRCS := $(wildcard $(SRCDIR)/*.cpp)
BIN_DIR := bin
TARGET := $(BIN_DIR)/minicc

.PHONY: all clean test

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(SRCS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

test: all
	@chmod +x ./scripts/test_examples.sh
	@./scripts/test_examples.sh
