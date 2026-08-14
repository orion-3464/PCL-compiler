.PHONY: clean distclean default libpcl

BUILD := build

CXX := g++
CXXFLAGS := -g -Wall -Wextra -Wpedantic -std=c++23 -O3 -I./src -I./$(BUILD) -I./src/frontend/ast

CC := gcc
CFLAGS := -O2 -Wall -Wextra -Wpedantic -std=c23 -I./src/libpcl -fno-builtin-trunc -fno-builtin-round

default: pcl libpcl

pcl: $(BUILD)/lexer.o $(BUILD)/parser.o $(BUILD)/ast.o $(BUILD)/error.o $(BUILD)/symbol.o $(BUILD)/main.o  
	$(CXX) $(CXXFLAGS) -o $@ $^

libpcl: $(BUILD)/libpcl.o

$(BUILD)/libpcl.o: src/libpcl/libpcl.c src/libpcl/libpcl.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/main.o: src/main.cpp $(BUILD)/parser.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/symbol.o: src/frontend/ast/symbol.cpp src/frontend/ast/symbol.hpp $(BUILD)/location.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/error.o: src/frontend/ast/error.cpp src/frontend/ast/error.hpp $(BUILD)/location.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/ast.o: src/frontend/ast/ast.cpp src/frontend/ast/ast.hpp $(BUILD)/location.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/parser.o: $(BUILD)/parser.cpp $(BUILD)/parser.hpp $(BUILD)/lexer.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/lexer.o: $(BUILD)/lexer.cpp $(BUILD)/lexer.hpp $(BUILD)/parser.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/parser.cpp $(BUILD)/parser.hpp $(BUILD)/location.hpp : src/frontend/parser.y | $(BUILD)
	bison -dv -o $(BUILD)/parser.cpp $< 

$(BUILD)/lexer.cpp $(BUILD)/lexer.hpp: src/frontend/lexer.l | $(BUILD)
	flex -s $< 

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

distclean: clean
	rm -f pcl
