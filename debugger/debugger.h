/*debugger.h*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Northwestern University
// CS 211
//

#pragma once

#include <string>
#include <map>
#include <unordered_set>

#include "programgraph.h"
#include "ram.h"

using namespace std;

class Debugger
{
private:
  string State;
  map<int, bool> Breakpoints;
  unordered_set<int> StatementLines; //stores the lines where there are stmts
  struct STMT* Program;
  struct RAM*  Memory;
  
  void printValue(string varname, struct RAM_VALUE* value);
  struct STMT* findStmt(struct STMT* cur, int lineNum);
  struct STMT* breakLink(struct STMT* cur);
  void restoreLink(struct STMT* cur, struct STMT* next);
  void programLength();

  


public:
  Debugger(struct STMT* program);

  ~Debugger();

  void run();
};

