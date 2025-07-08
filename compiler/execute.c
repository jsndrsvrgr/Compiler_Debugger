/*execute.c*/

//
// Execute program to parse and execute nupython code
//
// Jose Vergara
// Northwestern University
// CS211
// Winter 2025


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"
#include <math.h>

//
// Private functions:
//

// TODO:
// if you introduce any private functions (helper functions)
// declare them here with the keyword static,
// and implement them at the end of the file (still with keyword static)

//Executes a function call statement 
//
// Takes a statement structure and a memory sttrucutre
//
// Returns true if successful, false o/w
bool execute_function_call(struct STMT* stmt, struct RAM* memory);

//Helper function that evaluate print function call statement
//
// Takes a stmt struct and a memory struct
//
// Return true if successful, false o/w
static bool execute_print(struct STMT* stmt, struct RAM* memory);

//Executes an assignment statement 
//
// Takes a statement structure and a memory sttrucutre
//
// Returns true if successful, false o/w
bool execute_assignment(struct STMT* stmt, struct RAM* memory);

//Evaluates string operations for assignments and returns the result
//
// Takes left and right operands, the operator, and the line number
// for error reporting
//
// Returns a result structure containing success and ram value arguments.
static struct RESULT execute_string(struct RAM_VALUE lhs, struct RAM_VALUE rhs, int operator, int line);

//Evaluates integer operations for assignments and returns the result
//
// Takes left and right operands, the operator, and the line number
// for error reporting
//
// Returns a result structure containing success and value arguments.
static struct RESULT execute_int(struct RAM_VALUE lhs, struct RAM_VALUE rhs, int operator, int line);

//Evaluates floating point operations for assignments and returns the result
//
// Takes left and right operands, the operator, and the line number
// for error reporting
//
// Returns a result structure containing success and value arguments.
static struct RESULT execute_float(struct RAM_VALUE lhs, struct RAM_VALUE rhs, int operator, int line);

//Executes the input() function and returns the user input as a string
//
// Takes the prompt string and line number for error reporting
//
// Returns a result structure containing the input string
static struct RESULT execute_input_function(char* prompt, int line);

//Converts a string to an integer value
//
// Takes the string to convert and the line number for error reporting
//
// Returns a result structure containing the integer value if successful
static struct RESULT execute_int_function(char* s, int line);

//Converts a string to a float value
//
// Takes the string to convert and the line number for error reporting
//
// Returns a result structure containing the float value if successful
static struct RESULT execute_float_function(char* s, int line);


//Evaluates binary expression for assignments and returns the result
//
// Takes an expression structure, a memory strucutre, and the line of the expression
// as parameters
//
// Returns  a result strucutrue containing success and value arguments.
static struct RESULT execute_binary_expression(struct EXPR* expr, struct RAM* memory, int line);

// Retrieves the values of an element struct
//
//Takes a pointer to the element struct, memory struct, and the line number of the expression
// for error reporting.
//
// Returns  a result strucutrue containing success and value arguments.
static struct RESULT retrieve_value(struct ELEMENT* element, struct RAM* memory, int line);



struct RESULT
{
  bool success;
  struct RAM_VALUE ram_value;  

};
//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory)
{
  struct STMT* stmt = program;

  while(stmt != NULL) {
    if (stmt->stmt_type == STMT_ASSIGNMENT){
      int stmt_line = stmt->line;
      //printf("Line %d: assignment\n", stmt_line);
      if (execute_assignment(stmt, memory) == false){
        return;
      }
      stmt = stmt->types.assignment->next_stmt;
    }
    else if (stmt->stmt_type == STMT_FUNCTION_CALL){
      int stmt_line = stmt->line;
      //printf("Line %d: function call\n", stmt_line);
      if (execute_function_call(stmt, memory) == false){
        return;
      }
      stmt = stmt->types.function_call->next_stmt;
    }
    else if (stmt->stmt_type == STMT_WHILE_LOOP){
      struct EXPR* condition = stmt->types.while_loop->condition;
      struct RESULT condition_result;

        if (condition->isBinaryExpr) {
          condition_result = execute_binary_expression(condition, memory, stmt->line);
        } 
        else {
          // If not a binary expression, just retrieve the value of lhs
          condition_result = retrieve_value(condition->lhs->element, memory, stmt->line);
        }

      if(!condition_result.success){
        return;
      }
      if(condition_result.ram_value.types.i != 0){
        stmt = stmt->types.while_loop->loop_body;
      }
      else{
        stmt = stmt->types.while_loop->next_stmt;
      }
    }
    else{
      assert(stmt->stmt_type == STMT_PASS);
      int stmt_line = stmt->line;
      // printf("Line %d: pass\n", stmt_line);
      stmt = stmt->types.pass->next_stmt;
    }
  }
}

bool execute_function_call(struct STMT* stmt, struct RAM* memory) {
  if (stmt->stmt_type != STMT_FUNCTION_CALL) {
    return false;
  }
  //Allow for multiple function names
  char* function_name = stmt->types.function_call->function_name;

  if (strcmp(function_name, "print") == 0) {
    if (!execute_print(stmt, memory)){
      return false;
    }; //Executes print function
    return true;
  }
  return false;
//Allow handling for non recgnizable functions
//  printf("**SEMANTIC ERROR: function '%s' is not defined (line %d)\n", 
//         function_name, stmt->line);
//  return false;
}

// Helper function to execute print function calls
static bool execute_print(struct STMT* stmt, struct RAM* memory) {
  struct ELEMENT* parameter = stmt->types.function_call->parameter;
  
  if (parameter == NULL) {
    printf("\n"); // Print empty line if no parameter
    return true;
  }
  
  int element_type = parameter->element_type;
  char* value = parameter->element_value;
  
  if (element_type == ELEMENT_INT_LITERAL) {
    int int_value = atoi(value);
    printf("%d\n", int_value); // Print integer literal
  }
  else if (element_type == ELEMENT_IDENTIFIER) {
    struct RAM_VALUE* ram_value = ram_read_cell_by_name(memory, value);
    
    int line = stmt->line;
    if (ram_value == NULL) {
      // Error if variable is undefined
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", value, line);
      return false;
    }
    // Print element based on type
    if (ram_value->value_type == RAM_TYPE_INT) {
      printf("%d\n", ram_value->types.i);
    }
    else if (ram_value->value_type == RAM_TYPE_REAL) {
      printf("%f\n", ram_value->types.d);
    }
    else if (ram_value->value_type == RAM_TYPE_STR) {
      printf("%s\n", ram_value->types.s);
    }
    else if (ram_value->value_type == RAM_TYPE_BOOLEAN) {
      printf("%s\n", ram_value->types.i ? "True" : "False");
    }
  }
  else if (element_type == ELEMENT_REAL_LITERAL) {
    double real_value = atof(value);
    printf("%f\n", real_value); // Print Float value
  }
  else if (element_type == ELEMENT_STR_LITERAL) {
    printf("%s\n", value); // Print string value
  }
  else if (element_type == ELEMENT_TRUE || element_type == ELEMENT_FALSE) {
    printf("%s\n", value); // Print boolean value
  }
  else {
    printf("%s\n", value); // Print other literals
  }
  
  return true;
}

bool execute_assignment(struct STMT* stmt, struct RAM* memory){
  if(stmt->stmt_type == STMT_ASSIGNMENT){
    
    char* varname = stmt->types.assignment->var_name; // Get variable name
    if (stmt->types.assignment->rhs->value_type == VALUE_FUNCTION_CALL){
      char* function_name = stmt->types.assignment->rhs->types.function_call->function_name;
      
      struct RESULT function_result;
      function_result.success = false;
      if(strcmp(function_name, "input") == 0){
        char* input_prompt = stmt->types.assignment->rhs->types.function_call->parameter->element_value;
        function_result = execute_input_function(input_prompt, stmt->line);
      }
      else if(strcmp(function_name, "int") == 0){
        char* str_variable = stmt->types.assignment->rhs->types.function_call->parameter->element_value;
        struct RAM_VALUE* str_value = ram_read_cell_by_name(memory, str_variable);

        function_result = execute_int_function(str_value->types.s, stmt->line);

        if(!function_result.success){
          printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", stmt->line);
          return false;
        }

      }
      else if (strcmp(function_name, "float") == 0){
        char* str_variable = stmt->types.assignment->rhs->types.function_call->parameter->element_value;
        struct RAM_VALUE* str_value = ram_read_cell_by_name(memory, str_variable);

        function_result = execute_float_function(str_value->types.s, stmt->line);

        if(!function_result.success){
          printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", stmt->line);
          return false;
        }
      }
      if (ram_write_cell_by_name(memory, function_result.ram_value, varname) == false){
        return false;
      }
      return true;
    }
    else {
      struct EXPR* expr = stmt->types.assignment->rhs->types.expr; // rhs value
      

      struct RESULT result;
      
      if(expr->isBinaryExpr){
        result = execute_binary_expression(expr, memory, stmt->line);
      }
      else{
        result = retrieve_value(expr->lhs->element, memory, stmt->line);
      }
      if (!result.success){
        return false; 
      }
      //write value to memory cell
      if (ram_write_cell_by_name(memory, result.ram_value, varname) == false){
        return false; //Error writing to memory
      }
      return true;
    }
  return false;
  }
  return false;
}
static struct RESULT execute_input_function(char* prompt, int line){
  struct RESULT result;
  result.success = false;
  char lineBuffer[256];
  printf("%s", prompt);
  fgets(lineBuffer, sizeof(lineBuffer), stdin);
  lineBuffer[strcspn(lineBuffer, "\r\n")] = '\0';
  char* str_input = (char*) malloc(strlen(lineBuffer) + 1);

  strcpy(str_input, lineBuffer);
  result.ram_value.value_type = RAM_TYPE_STR;
  result.ram_value.types.s = str_input;
  return result;
}

static struct RESULT execute_int_function(char* s, int line){
  struct RESULT result;
  result.success = false;
  int convert = atoi(s);
  int i = 0;
  bool valid = true;
  if (convert != 0){
    valid = true;
  }
  else{
    for (int i = 0; s[i] != '\0'; i++) {
    if (s[i] != '0' && s[i] != '.')
        valid = false;
    }
  }
  if (valid){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_INT;
    result.ram_value.types.i = convert;
    return result;
  }
  return result;

}
static struct RESULT execute_float_function(char* s, int line){
  struct RESULT result;
  result.success = false;
  double convert = atof(s);
  int i = 0;
  bool valid = true;
  if (convert != 0){
    valid = true;
  }
  else{
    for (int i = 0; s[i] != '\0'; i++) {
    if (s[i] != '0' && s[i] != '.')
        valid = false;
    }
  }
  if (valid){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_REAL;
    result.ram_value.types.d = convert;
    return result;
  }
  return result;
}

static struct RESULT execute_binary_expression(struct EXPR* expr, struct RAM* memory, int line){
  
  struct RESULT result;
  result.success = false; //Init as unsuccesful expression

  if (expr == NULL){
    return result;
  }
  struct RESULT lhs = retrieve_value(expr->lhs->element, memory, line);
  
  if(!lhs.success){
    return result;
  }

  if (expr->operator != OPERATOR_NO_OP && expr->rhs != NULL){
    //binary operation
    struct RESULT rhs = retrieve_value(expr->rhs->element, memory, line);
    if(!rhs.success){
      return result;
    }

    if (lhs.ram_value.value_type == RAM_TYPE_STR && rhs.ram_value.value_type == RAM_TYPE_STR){
        return execute_string(lhs.ram_value, rhs.ram_value, expr->operator, line);
    }

    else if (lhs.ram_value.value_type == RAM_TYPE_INT && rhs.ram_value.value_type == RAM_TYPE_INT){
    return execute_int(lhs.ram_value, rhs.ram_value, expr->operator, line);
    }

    else if((lhs.ram_value.value_type == RAM_TYPE_REAL && rhs.ram_value.value_type == RAM_TYPE_REAL) || 
        (lhs.ram_value.value_type == RAM_TYPE_REAL && rhs.ram_value.value_type == RAM_TYPE_INT) ||
        (lhs.ram_value.value_type == RAM_TYPE_INT && rhs.ram_value.value_type == RAM_TYPE_REAL)){
      
      return execute_float(lhs.ram_value, rhs.ram_value, expr->operator, line);
    }
    printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
    return result;
  }
  return result;
}

static struct RESULT execute_string(struct RAM_VALUE lhs, struct RAM_VALUE rhs, int operator, int line){
  struct RESULT result;
  result.success = false;

  if (operator == OPERATOR_PLUS){

    char* concat_string = (char*) malloc(strlen(lhs.types.s) + strlen(rhs.types.s) + 1);

    if (concat_string == NULL){
      printf("**Segmentation Fault: memory allocation failed (line %d)\n", line);
      return result;
    }

    strcpy(concat_string, lhs.types.s);
    strcat(concat_string, rhs.types.s);

    result.ram_value.value_type = RAM_TYPE_STR;
    result.ram_value.types.s = concat_string;
    result.success = true;
    return result;
  }
  else if (operator == OPERATOR_EQUAL || operator == OPERATOR_NOT_EQUAL ||
    operator == OPERATOR_LT || operator == OPERATOR_LTE || operator == OPERATOR_GT ||
    operator == OPERATOR_GTE){
    
    int compare = strcmp(lhs.types.s, rhs.types.s);
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;

    if (operator == OPERATOR_EQUAL){
        result.ram_value.types.i = (compare == 0) ? 1 : 0;
        }
    else if (operator == OPERATOR_NOT_EQUAL){
        result.ram_value.types.i = (compare != 0) ? 1 : 0;
        }
    else if (operator == OPERATOR_LT){
        result.ram_value.types.i = (compare < 0) ? 1 : 0;
        }
    else if (operator == OPERATOR_LTE){
        result.ram_value.types.i = (compare <= 0) ? 1 : 0;
        }
    else if (operator == OPERATOR_GT){
        result.ram_value.types.i = (compare > 0) ? 1 : 0;
        }
    else if (operator == OPERATOR_GTE){
        result.ram_value.types.i = (compare >= 0) ? 1 : 0;
        }
    result.success = true;
    return result;
  }

 printf("**SEMANTIC ERROR: invalid operand for string (line %d)\n", line);
 return result;
}
static struct RESULT execute_int(struct RAM_VALUE lhs, struct RAM_VALUE rhs, int operator, int line){
  struct RESULT result;
  result.success = false;
  result.ram_value.value_type = RAM_TYPE_INT;  

  int lhs_value = lhs.types.i;
  int rhs_value = rhs.types.i;
    //Perform operations based on the operator type
  if (operator == OPERATOR_PLUS){ // Addition
    
    result.success = true;      
    result.ram_value.types.i = lhs_value + rhs_value;
    return result;
  }
  else if (operator == OPERATOR_MINUS){ // Substraction
    result.success = true;        
    result.ram_value.types.i = lhs_value - rhs_value;
    return result;
  }
  else if(operator== OPERATOR_ASTERISK){//Multiplication
    result.success = true;
    result.ram_value.types.i = lhs_value * rhs_value;
    return result;
  }
  else if(operator == OPERATOR_MOD){ // Modulo
    if (rhs_value == 0)
    {//Handle division by 0
      printf( "**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = false;
      return result;
    }
    result.success = true;
    result.ram_value.types.i = lhs_value % rhs_value;
    return result;
  }
  else if(operator == OPERATOR_DIV){ //Division
    if (rhs_value == 0)
    {//Handle division by 0
      printf( "**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = false;
      return result;
    }
    result.success = true;
    result.ram_value.types.i = lhs_value / rhs_value;
    return result;
  }
  else if(operator == OPERATOR_POWER){ //Power
    result.success = true;
    result.ram_value.types.i = pow(lhs_value, rhs_value);
    return result;
  }
  
  else if(operator == OPERATOR_NO_OP){//Just the left hand side
    result.success = true;
    result.ram_value.types.i = lhs_value;
    return result;
  }//Relational Operators
  else if(operator == OPERATOR_EQUAL){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value == rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_NOT_EQUAL){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value != rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_LT){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value < rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_LTE){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value <= rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_GT){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value > rhs_value) ? 1 : 0;
    return result;
  }
  else if (operator == OPERATOR_GTE){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value >= rhs_value) ? 1 : 0;
    return result;
  }
  result.success = false;
  return result; 
}

static struct RESULT execute_float(struct RAM_VALUE lhs, struct RAM_VALUE rhs, int operator, int line){
  struct RESULT result;
  result.success = false;
  result.ram_value.value_type = RAM_TYPE_REAL;
  double lhs_value;
  double rhs_value;

  lhs_value = (lhs.value_type == RAM_TYPE_INT) ? (float) lhs.types.i : lhs.types.d;
  rhs_value = (rhs.value_type == RAM_TYPE_INT) ? (float) rhs.types.i : rhs.types.d;

  if (operator == OPERATOR_PLUS){ // Addition
    result.success = true;      
    result.ram_value.types.d = lhs_value + rhs_value;
    return result;
  }
  else if (operator == OPERATOR_MINUS){ // Substraction
    result.success = true;        
    result.ram_value.types.d = lhs_value - rhs_value;
    return result;
  }
  else if(operator== OPERATOR_ASTERISK){//Multiplication
    result.success = true;
    result.ram_value.types.d = lhs_value * rhs_value;
    return result;
  }
  else if(operator == OPERATOR_MOD){ // Modulo
    if (rhs_value == 0)
    {//Handle division by 0
      printf( "**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = false;
      return result;
    }
    result.success = true;
    result.ram_value.types.d = fmod(lhs_value, rhs_value);
    return result;
  }
  else if(operator == OPERATOR_DIV){ //Division
    if (rhs_value == 0)
    {//Handle division by 0
      printf( "**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = false;
      return result;
    }
    result.success = true;
    result.ram_value.types.d = lhs_value / rhs_value;
    return result;
  }
  else if(operator == OPERATOR_POWER){ //Power

    result.success = true;
    result.ram_value.types.d = pow(lhs_value, rhs_value);
    return result;
  }
  else if(operator == OPERATOR_NO_OP){//Just the left hand side
    result.success = true;
    result.ram_value.types.i = lhs_value;
    return result;
  }//Relational Operators
  else if(operator == OPERATOR_EQUAL){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value == rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_NOT_EQUAL){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value != rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_LT){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value < rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_LTE){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value <= rhs_value) ? 1 : 0;
    return result;
  }
  else if(operator == OPERATOR_GT){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value > rhs_value) ? 1 : 0;
    return result;
  }
  else if (operator == OPERATOR_GTE){
    result.success = true;
    result.ram_value.value_type = RAM_TYPE_BOOLEAN;
    result.ram_value.types.i = (lhs_value >= rhs_value) ? 1 : 0;
    return result;
  }
  result.success = false;
  return result; 
}


static struct RESULT retrieve_value(struct ELEMENT* element, struct RAM* memory, int line){

  struct RESULT result;
  result.success = false; //Initialize unsuccessful

  char* value = element->element_value; //Get the element value string

  struct RAM_VALUE* varvalue = ram_read_cell_by_name(memory, value); // Read from memory 

  if (element->element_type == ELEMENT_INT_LITERAL){//Integer literal case
    result.success = true;
    result.ram_value.types.i = atoi(value);
    result.ram_value.value_type = RAM_TYPE_INT;
    return result;
  }
  else if(element->element_type == ELEMENT_IDENTIFIER){//Handle variable identifier
    if(varvalue == NULL){
      //Error if variable is undefined
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", value, line);
      return result;
    }
    result.success = true;
    result.ram_value.value_type = varvalue->value_type;

    if(varvalue->value_type == RAM_TYPE_INT || varvalue->value_type == RAM_TYPE_BOOLEAN){
      result.ram_value.types.i = varvalue->types.i;
    }
    else if (varvalue->value_type == RAM_TYPE_REAL){
      result.ram_value.types.d = varvalue->types.d;
    }
    else if (varvalue->value_type == RAM_TYPE_STR){
      result.ram_value.types.s = varvalue->types.s;
    }
    return result;
  }
  else if(element->element_type == ELEMENT_REAL_LITERAL){
    result.success = true;
    result.ram_value.types.d = atof(value);
    result.ram_value.value_type = RAM_TYPE_REAL;
    return result;
  }
  else if(element->element_type == ELEMENT_STR_LITERAL){
    result.success = true;
    result.ram_value.types.s = value;
    result.ram_value.value_type = RAM_TYPE_STR;
    return result;
  }
  else if(element->element_type == ELEMENT_FALSE || element->element_type == ELEMENT_TRUE){
    if(element->element_type == ELEMENT_FALSE){
      result.success = true;
      result.ram_value.types.i = 0;
      result.ram_value.value_type = RAM_TYPE_BOOLEAN;
      return result;
    }
    else{
      result.success = true;
      result.ram_value.types.i = 1;
      result.ram_value.value_type = RAM_TYPE_BOOLEAN;
      return result;
    }
  }

  return result;
}


