CC := gcc
CFLAGS := -g -Wall -Wextra -std=c++23
BUILD_DIR := ./build

lexer: lexer.cpp
	$(CC) $(CFLAGS) -o $@ $(BUILD_DIR)/lexer.cpp && mv $@ $(BUILD_DIR)

lexer.cpp: $(BUILD_DIR)
	flex -s src/lexer.l && mv $@ $(BUILD_DIR) 

$(BUILD_DIR):
	mkdir $@

.PHONY: clean
clean:
	rm -rf build/
