/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for multiple breakpoints and step-by-step
// execution of straight-line code. Uses nuPython interpreter as 
// execution engine.
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>

#include "debugger.h"
#include "execute.h"
#include "programgraph.h"
#include "tokenqueue.h"

using namespace std;

//
// printValue
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::printValue(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// breakLink
//
// Breaks the link between the cur statement and the one 
// that follows; returns a pointer to the statement that
// follows before the link was broken.
//
// NOTE: with a while loop, we break both links.
//
struct STMT* Debugger::breakLink(struct STMT* cur)
{
  STMT* next = nullptr;
  
  if (cur == nullptr) // nothing to break:
    return nullptr;
  
  if (cur->stmt_type == STMT_ASSIGNMENT) {
    next = cur->types.assignment->next_stmt;
    cur->types.assignment->next_stmt = nullptr;
  }
  else if (cur->stmt_type == STMT_FUNCTION_CALL) {
    next = cur->types.function_call->next_stmt;
    cur->types.function_call->next_stmt = nullptr;
  }
  else if (cur->stmt_type == STMT_PASS) {
    next = cur->types.pass->next_stmt;
    cur->types.pass->next_stmt = nullptr;
  }
  else if (cur->stmt_type == STMT_WHILE_LOOP){//Breaklink with next not loop body
    next = cur->types.while_loop->next_stmt;
    cur->types.while_loop->next_stmt = nullptr;
  }
  else { // unexpected stmt type:
    cout << "**ERROR" << endl;
    cout << "**ERROR: unexpected stmt type in breakLink" << endl;
    cout << "**ERROR" << endl;
    assert(false);
  }
  
  return next;
}


//
// restoreLink
//
// Restores the link between the cur statement and the one 
// that follows.
//
void Debugger::restoreLink(struct STMT* cur, struct STMT* next)
{
  if (cur == nullptr) // nothing to restore:
    return;
  
  if (cur->stmt_type == STMT_ASSIGNMENT) {
    cur->types.assignment->next_stmt = next;
  }
  else if (cur->stmt_type == STMT_FUNCTION_CALL) {
    cur->types.function_call->next_stmt = next;
  }
  else if (cur->stmt_type == STMT_PASS) {
    cur->types.pass->next_stmt = next;
  }
  else if (cur->stmt_type == STMT_WHILE_LOOP){
    cur->types.while_loop->next_stmt = next;
  }
  else { // unexpected stmt type:
    cout << "**ERROR" << endl;
    cout << "**ERROR: unexpected stmt type in restoreLink" << endl;
    cout << "**ERROR" << endl;
    assert(false);
  }
  
  return;
}

//
//Find the length of the program (Number of nodes)
//
void Debugger::programLength()
{

  struct STMT* stmt = this->Program;
  while(stmt != nullptr){//Go through the whole program nodes

    if (stmt->stmt_type == STMT_ASSIGNMENT) {
      StatementLines.emplace(stmt->line); //Emplace line int
      stmt = stmt->types.assignment->next_stmt;
    }
    else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      StatementLines.emplace(stmt->line);
      stmt = stmt->types.function_call->next_stmt;
    }
    else if (stmt->stmt_type == STMT_PASS) {
      StatementLines.emplace(stmt->line);
      stmt = stmt->types.pass->next_stmt;
    }    
    else if (stmt->stmt_type == STMT_WHILE_LOOP) {
      auto iter = this->StatementLines.find(stmt->line);//Have we gone through this node
      if(iter == StatementLines.end()){//If we havent emplace the line number
        StatementLines.emplace(stmt->line);
        stmt = stmt->types.while_loop->loop_body; //Go through the body
      }
      else{//If we have go through the next stmts
        stmt = stmt->types.while_loop->next_stmt; 
      } 
    }    
    else { // unexpected stmt type:
    cout << "**ERROR" << endl;
    cout << "**ERROR: unexpected stmt type in restoreLink" << endl;
    cout << "**ERROR" << endl;
    assert(false);
    }
  }//while
}
//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr)
{
  this->Memory = ram_init();
}


//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;
  programLength(); //Find the length before breaking

  //
  // controls where we start execution from:
  //
  struct STMT* curStmt = this->Program;
  
  //
  // we're going to execute stmt-by-stmt, so we have to break
  // the program graph so that if we run the program, we just
  // run the first stmt:
  //
  struct STMT* nextStmt = breakLink(curStmt);


  //
  // command loop until quit is entered:
  //
  while (true) {
    
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > " << endl;
    cin >> cmd;
    
    if (cmd == "q") {
      break;
    }
    else if (cmd == "h") {
      
      cout << "Available commands:"
      << endl << "r -> Run the program / continue from a breakpoint"
      << endl << "s -> Step to next stmt by executing current stmt"
      << endl << "b n -> Breakpoint at line n"
      << endl << "rb n -> Remove breakpoint at line n"
      << endl << "lb -> List all breakpoints"
      << endl << "cb -> Clear all breakpoints"
      << endl << "p varname -> Print variable"
      << endl << "sm -> Show memory contents"
      << endl << "ss -> Show state of debugger"
      << endl << "w -> What line are we on?"
      << endl << "q -> Quit the debugger"
      << endl;
    }
    else if (cmd == "r" || cmd == "s") {
      
      //
      // run, or continue running, the program:
      //
      if (this->State == "Completed") {
        cout << "program has completed" << endl;
        continue; // skip the code below and repeat the loop for next cmd:
      }
      
      if (this->State == "Loaded")
        this->State = "Running";
      

      //
      // execute current stmt via nuPython interpreter:
      //
      while (curStmt != nullptr) {
        //
        //Check for breakpoint
        //
        auto iter = this->Breakpoints.find(curStmt->line);
        if (iter != Breakpoints.end()){ //Is the current statement a breakpoint?
          if (iter->second == false){ //Hit a breakpoint first time
            cout << "hit breakpoint at line " << iter->first << endl; //call programgraph_print() for debugging
            iter->second = true; //We have hit the breakpont now
            break;
          }
          iter->second = false; //Clear for the while loop
        }

        if (curStmt->stmt_type == STMT_WHILE_LOOP){
          //
          //Execute While Loop Statement
          //
          struct RAM_VALUE* value = execute_expr(curStmt, this->Memory, curStmt->types.while_loop->condition);
          
          if (value == nullptr) { // there was an error, we've complete execution:
            this->State = "Completed";
            // we need to repair the program graph:
            restoreLink(curStmt, nextStmt);
            break;
          }
          restoreLink(curStmt, nextStmt);
          if(value->types.i == 1){//Statement in while loop is true
            curStmt = curStmt->types.while_loop->loop_body;
            nextStmt = breakLink(curStmt);
          }
          else{
            curStmt = nextStmt;
            nextStmt = breakLink(curStmt);
          }
          ram_free_value(value);
        }
        else{
        // 
        // execute this line:
        //
          struct ExecuteResult er = execute(curStmt, this->Memory);
        
          //
          // what happened during execution?
          //
          if (!er.Success) { // there was an error, we've complete execution:
            this->State = "Completed";
            
            // we need to repair the program graph:
            restoreLink(curStmt, nextStmt);
            
            break;
          }
          //
          // advance one stmt:
          //
          restoreLink(curStmt, nextStmt);
          
          curStmt = nextStmt;
          nextStmt = breakLink(curStmt);
        }
        // 
        // are we stepping? if so, exit loop:
        //
        if (cmd== "s")
          break;
        
      }//while
      
      //
      // loop has ended, why? There are 3 cases:
      //   1. hit a breakpoint or we stepped once
      //   2. semantic error
      //   3. ran to completion
      //
      if (curStmt == nullptr) {  // we ran to completion:
        this->State = "Completed";
        // program graph is fine since we ran to the end
      }
      else if (this->State == "Completed") {  // semantic error
        //
        // handled inside loop
        //
      }
      else {
        //
        // else we did step, nothing to do here
        //
      }
    }
    else if(cmd == "b"){
      int line;
      cin >> line;
      auto iter = StatementLines.find(line);
      //calculate length of the program
      if (iter == StatementLines.end()){//check if there is a stmt in that line
        cout << "no such line" << endl;
      }
      else{
      auto iter = this->Breakpoints.find(line);
      if(iter != Breakpoints.end()){//breakpoint found
        cout << "breakpoint already set" << endl;
      }
      else if (iter == Breakpoints.end()){
        Breakpoints.emplace(line, false); //new breakpoint
        cout << "breakpoint set" << endl;
      }
      }
    }
    else if(cmd == "rb"){
      int line;
      cin >> line;
      auto iter = this->Breakpoints.find(line);
      //Lookup breakpoint
      if(iter == Breakpoints.end()){
        cout << "no such breakpoint" << endl;
      }
      else if (iter != Breakpoints.end()){
        Breakpoints.erase(iter); //Erase breakpoint if found
        cout << "breakpoint removed" << endl;
      }
    }
    else if (cmd == "lb"){
      auto iter = Breakpoints.begin();
      if (iter == Breakpoints.end()){//No breakpoints
        cout << "no breakpoints" << endl;
      }
      else{ 
        cout << "breakpoints on lines:";         
        while(iter != Breakpoints.end()){//Iterator loop
          cout << " " << iter->first;
          iter++;
        }
        cout << endl;
      }
    }
    else if(cmd == "cb"){
      Breakpoints.clear(); //Delete all breakpoints
      cout << "breakpoints cleared" << endl;
    }
    else if (cmd == "p") {
      
      string varname;
      cin >> varname;
      
      const char* name = varname.c_str();
      
      struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
      
      if (value == nullptr) {
        cout << "no such variable" << endl;
        continue;  // skip code below and continue with next cmd:
      }
      
      printValue(varname, value);
      ram_free_value(value);
    }
    else if (cmd == "sm") {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") {
      
      if (this->State == "Loaded") {
        cout << "line " << curStmt->line << endl;
        // programgraph_print(curStmt);
      }
      else if (this->State == "Completed") {
        cout << "completed execution" << endl;
      }
      else { // we are running:
        cout << "line " << curStmt->line << endl;
        // programgraph_print(curStmt);
      }
    }
    else {
      
      cout << "unknown command" << endl;
    }
    
  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). Repair the program graph if need be:
  //
  if (curStmt != nullptr) {
    restoreLink(curStmt, nextStmt);
  }
  
}//run

