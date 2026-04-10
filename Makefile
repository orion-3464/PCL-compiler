.PHONY: clean distclean default

BUILD := build

CXX := g++
CXXFLAGS := -g -Wall -Wextra -Wpedantic -std=c++23 -O3 -I./src -I./$(BUILD)

default: pcl

pcl: $(BUILD)/lexer.o $(BUILD)/parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/parser.o: $(BUILD)/parser.cpp $(BUILD)/parser.hpp $(BUILD)/lexer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/lexer.o: $(BUILD)/lexer.cpp $(BUILD)/lexer.hpp $(BUILD)/parser.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/parser.cpp $(BUILD)/parser.hpp: src/parser.y | $(BUILD)
	bison -dv -o $(BUILD)/parser.cpp $< 

$(BUILD)/lexer.cpp: src/lexer.l | $(BUILD)
	flex -s -o $@ $< 

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

distclean: clean
	rm -f pcl
