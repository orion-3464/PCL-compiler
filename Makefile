.PHONY: clean distclean default 

BUILD := build

LLVM_CXXFLAGS := $(shell llvm-config-19 --cxxflags)
LLVM_LDLFLAGS := $(shell llvm-config-19 --ldflags --system-libs --libs core)

CXX := clang++
CXXFLAGS := -g -Wall -Wextra -Wpedantic -Wno-unused-parameter -O3 -I./src -I./$(BUILD) $(LLVM_CXXFLAGS)

CC := clang
CFLAGS := -g -Wall -Wextra -Wpedantic -std=c23 -O2 -I./src/libpcl -fno-builtin-trunc -fno-builtin-round

ifndef V
	Q := @
else
	Q :=
endif

default: pclc $(BUILD)/libpcl.a

pclc: $(BUILD)/lexer.o $(BUILD)/parser.o $(BUILD)/ast.o $(BUILD)/error.o $(BUILD)/symbol.o $(BUILD)/codegen.o $(BUILD)/main.o
	@echo "[LINK] Building compiler executable: $@"
	$(Q)$(CXX) $(CXXFLAGS) -rdynamic -o $@ $^ $(LLVM_LDLFLAGS) 

$(BUILD)/libpcl.a: $(BUILD)/libpcl.o | $(BUILD)
	@echo "[AR] Archiving standard library: $@"
	$(Q)$(AR) rcs $@ $^

$(BUILD)/libpcl.o: src/libpcl/libpcl.c src/libpcl/libpcl.h | $(BUILD)
	@echo "[CC] Compiling library: $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/main.o: src/main.cpp $(BUILD)/parser.hpp | $(BUILD)
	@echo "[CXX] Compiling: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/codegen.o: src/backend/codegen.cpp src/backend/codegen.hpp | $(BUILD)
	@echo "[CXX] Compiling: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/symbol.o: src/ast/symbol.cpp src/ast/symbol.hpp $(BUILD)/location.hpp | $(BUILD)
	@echo "[CXX] Compiling: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/error.o: src/ast/error.cpp src/ast/error.hpp $(BUILD)/location.hpp | $(BUILD)
	@echo "[CXX] Compiling: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/ast.o: src/ast/ast.cpp src/ast/ast.hpp $(BUILD)/location.hpp | $(BUILD)
	@echo "[CXX] Compiling: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(BUILD)/parser.o: $(BUILD)/parser.cpp $(BUILD)/parser.hpp $(BUILD)/lexer.hpp | $(BUILD)
	@echo "[CXX] Compiling Bison output: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/lexer.o: $(BUILD)/lexer.cpp $(BUILD)/lexer.hpp $(BUILD)/parser.hpp | $(BUILD)
	@echo "[CXX] Compiling Flex output: $<"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/parser.cpp $(BUILD)/parser.hpp $(BUILD)/location.hpp : src/frontend/parser.y | $(BUILD)
	@echo "[BISON] Generating parser from $<"
	$(Q)bison -dv -o $(BUILD)/parser.cpp $< 

$(BUILD)/lexer.cpp $(BUILD)/lexer.hpp: src/frontend/lexer.l | $(BUILD)
	@echo "[FLEX] Generating lexer from $<"
	$(Q)flex -s $< 

$(BUILD):
	$(Q)mkdir -p $(BUILD)

clean:
	@echo "[CLEAN] Removing build directory"
	$(Q)rm -rf $(BUILD)

distclean: clean
	@echo "[CLEAN] Removing executable"
	$(Q)rm -f pclc
