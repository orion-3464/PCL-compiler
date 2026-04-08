.PHONY: clean distclean default

CXX := g++
CXXFLAGS := -g -Wall -Wextra -std=c++23 -O3 -I./src -I./$(BUILD)
BUILD := build

default: pcl

pcl: $(BUILD)/lexer.o $(BUILD)/parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/parser.o: $(BUILD)/parser.cpp $(BUILD)/parser.hpp src/lexer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/lexer.o: $(BUILD)/lexer.cpp src/lexer.hpp $(BUILD)/parser.hpp
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
