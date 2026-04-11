.PHONY: clean distclean default

BUILD := build

CXX := g++
CXXFLAGS := -g -Wall -Wextra -Wpedantic -std=c++23 -O3 -I./src -I./$(BUILD) -I./src/ast

default: pcl

pcl: $(BUILD)/lexer.o $(BUILD)/parser.o $(BUILD)/ast.o $(BUILD)/main.o  
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/main.o: src/main.cpp $(BUILD)/parser.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/ast.o: src/ast/ast.cpp src/ast/ast.hpp $(BUILD)/location.hpp | $(BUILD)
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
