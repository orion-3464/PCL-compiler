.PHONY: clean distclean default

CXX := g++
CXXFLAGS := -g -Wall -Wextra -std=c++23 -o3
BUILD := build

default: pcl

pcl: $(BUILD)/parser.o $(BUILD)/lexer.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/parser.o: $(BUILD)/parser.cpp $(BUILD)/parser.hpp src/lexer.hpp
	$(CXX) $(CXXFLAGS) -c -I./src $< -o $@

$(BUILD)/lexer.o: $(BUILD)/lexer.cpp src/lexer.hpp
	$(CXX) $(CXXFLAGS) -c -I./src $< -o $@

$(BUILD)/parser.cpp $(BUILD)/parser.hpp: src/parser.y | $(BUILD)
	bison -dv -o parser.cpp $< && mv parser* $(BUILD)

$(BUILD)/lexer.cpp: src/lexer.l | $(BUILD)
	flex -s $< && mv lexer.cpp $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

distclean: clean
	rm -f pcl
