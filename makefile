# Top-level Makefile for Compiler-Debugger project
#
#   make compiler   → builds ./compiler_out
#   make debugger   → builds ./debugger_out
#   make tests      → builds & runs ./ram_tests
#   make clean      → removes only what we generated
#

CC       := gcc
CXX      := g++
CFLAGS   := -std=c11 -g -Wall -pedantic -Werror -Icompiler -Wno-unused-variable -Wno-unused-function 
CXXFLAGS := -std=c++17 -g -Wall -pedantic -Werror -I. -lm -Wno-unused-variable -Wno-unused-function 

.PHONY: all compiler debugger tests clean

all: compiler debugger tests

# -----------------------------------------------------------------------------
# 1) Build the interpreter/compiler
# -----------------------------------------------------------------------------
compiler_out: \
    compiler/main.c \
    compiler/ram.c \
    compiler/execute.c \
    compiler/programgraph.o \
    compiler/parser.o \
    compiler/scanner.o \
    compiler/tokenqueue.o
	$(CC) $(CFLAGS) $^ -no-pie -o $@

compiler: compiler_out

# -----------------------------------------------------------------------------
# 2) Build the debugger
# -----------------------------------------------------------------------------
debugger_out: \
    debugger/main.o       \
    debugger/debugger.cpp \
    compiler/ram.c        \
    compiler/execute.c    \
    compiler/programgraph.o \
    compiler/parser.o       \
    compiler/scanner.o      \
    compiler/tokenqueue.o
	$(CXX) $(CXXFLAGS) $^ -o $@

debugger: debugger_out

# -----------------------------------------------------------------------------
# 3) Build & run RAM unit-tests (Google Test)
# -----------------------------------------------------------------------------
ram_tests: \
	tests/main.c 	\
	tests/gtest.o  \
    tests/tests.c     \
    compiler/ram.c 	 
	$(CXX) $(CXXFLAGS) $^ -lpthread -o $@
	@./ram_tests

tests: ram_tests

# -----------------------------------------------------------------------------
# 4) Clean up exactly the files we generated
# -----------------------------------------------------------------------------
clean:
	rm -f compiler_out debugger_out ram_tests
