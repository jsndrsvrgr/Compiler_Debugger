
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h>

#include "ram.h"


//
// Public functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM* ram_init(void)
{
  struct RAM* ram = (struct RAM*) malloc(sizeof(struct RAM));
  if(ram ==  NULL){
    fprintf(stderr, "Error: Failed to allocate memory for RAM"); //If we couldnt allocate memory then error
    return NULL;
  }
  ram->capacity = 4;
  ram->num_values = 0;
  ram->cells = (struct RAM_CELL*) malloc(ram->capacity * sizeof(struct RAM_CELL));
  for(int i = 0; i < ram->capacity; i++){ //Initialize each cell in the ram array
    ram->cells[i].identifier = NULL;
    ram->cells[i].value.value_type = RAM_TYPE_NONE;
  }
  return ram;
}


//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory.
//
void ram_destroy(struct RAM* memory)
{
  for(int i = 0; i < memory->num_values; i++){//go through all of the assigned cells
    free(memory->cells[i].identifier);//Free the identifier strings
    memory->cells[i].identifier = NULL;//Set the identifiers to NULL
    if(memory->cells[i].value.value_type == RAM_TYPE_STR){//If the value holds a string
      free(memory->cells[i].value.types.s);//Free the string array
      memory->cells[i].value.types.s = NULL;//Set the pointer to null
    }
  }
  free(memory->cells);
  free(memory);
}


//
// ram_get_addr
// 
// If the given identifier (e.g. "x") has been written to 
// memory by name, returns the address of this value --- an integer
// in the range 0..N-1 where N is the number of values currently 
// stored in memory. Returns -1 if no such identifier exists 
// in memory. 
// 
// NOTE: a variable has to be written to memory by name before you can
// get its address. Once a variable is written to memory, its
// address never changes.
//
int ram_get_addr(struct RAM* memory, char* identifier)
{
  for(int i = 0; i < memory->num_values; i++){
    if(strcmp(memory->cells[i].identifier, identifier) == 0){//Are the strings equal?
      return i;
    }
  }
  return -1; // No match
}


//
// ram_read_cell_by_addr
//
// Given a memory address (an integer in the range 0..N-1), 
// returns a COPY of the value contained in that memory cell.
// Returns NULL if the address is not valid.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
// NOTE: a variable has to be written to memory by name before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address)
{
  if(address >= memory->num_values || address < 0) //out of bounds
    return NULL;
  
  struct RAM_VALUE* value_copy = (struct RAM_VALUE*) malloc(sizeof(struct RAM_VALUE)); //create copy of RAM_VALUE struct
  value_copy->value_type = memory->cells[address].value.value_type;
   if(value_copy->value_type == RAM_TYPE_STR){//If its a type string dupe 
    value_copy->types.s = strdup(memory->cells[address].value.types.s);
   }
   else
   {
    value_copy->types = memory->cells[address].value.types;
   }
   return value_copy;
}


// 
// ram_read_cell_by_name
//
// If the given name (e.g. "x") has been written to 
// memory, returns a COPY of the value contained in memory.
// Returns NULL if no such name exists in memory.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name)
{
  int address = ram_get_addr(memory, name);
  if (address == -1){
    return NULL;
  }
  return ram_read_cell_by_addr(memory, address);
}


//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_name and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE* value)
{
  if(value->value_type == RAM_TYPE_STR){
    free(value->types.s);
    value->types.s = NULL;
  }
  free(value);
}


//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given 
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if 
// the value was successfully written, false if not (which 
// implies the memory address is invalid).
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory by name before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address)
{
  if(address >= memory->num_values || address < 0){
    return false;
  }
  struct RAM_CELL* cell = &memory->cells[address]; //Get a pointer to the cell

  if(cell->value.value_type == RAM_TYPE_STR && cell->value.types.s != NULL){
    free(cell->value.types.s);//If its a string free the old one
    cell->value.types.s = NULL;
  }
  cell->value.value_type = value.value_type;//copy the new value_type
  
  if(cell->value.value_type == RAM_TYPE_STR){
    cell->value.types.s = strdup(value.types.s); //Duplicate string and assign
  }
  else
  {
    cell->value.types = value.types; //Update every other value type
  }
  return true;
}


//
// ram_write_cell_by_name
//
// Writes the given value to a memory cell named by the given
// name. If a memory cell already exists with this name, the
// existing value is overwritten by this new value. Returns
// true since this operation always succeeds.
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory by name before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name)
{
  int address = ram_get_addr(memory, name);
  if(address == -1){ //The variable does not currently exist in the array

    if(memory->capacity == memory->num_values){
      int old_capacity = memory->capacity;
      memory->capacity = memory->capacity * 2;
      struct RAM_CELL* new_cells = (struct RAM_CELL*) realloc(memory->cells, sizeof(struct RAM_CELL) * memory->capacity);
      memory->cells = new_cells;
      for(int i = old_capacity; i < memory->capacity; i++){
        memory->cells[i].identifier = NULL; //Initialize all of the identifiers to nullptrs
        memory->cells[i].value.value_type = RAM_TYPE_NONE;//initialize all of the new cells to none values
      }
    }
    address = memory->num_values;
    memory->cells[memory->num_values].identifier = strdup(name);//Duplicate the string into the new cell
    memory->cells[memory->num_values].value.value_type = RAM_TYPE_NONE; //Marking the current cell as optional in case
    memory->num_values++; //Updating the number of values in the array
  }
  return ram_write_cell_by_addr(memory, value, address);
}


//
// ram_print
//
// Prints the contents of memory to the console.
//
void ram_print(struct RAM* memory)
{
  printf("**MEMORY PRINT**\n");

  printf("Capacity: %d\n", memory->capacity);
  printf("Num values: %d\n", memory->num_values);
  printf("Contents:\n");

  for (int i = 0; i < memory->num_values; i++)
  {
      printf(" %d: %s, ", i, memory->cells[i].identifier);
      int value_type = memory->cells[i].value.value_type;
      if(value_type == RAM_TYPE_INT){
        printf("int, %d", memory->cells[i].value.types.i);        
      }
      else if (value_type == RAM_TYPE_REAL){
        printf("real, %lf", memory->cells[i].value.types.d);
      }
      else if(value_type == RAM_TYPE_STR){
        printf("str, '%s'", memory->cells[i].value.types.s);
      }
      else if(value_type == RAM_TYPE_PTR){
        printf("ptr, %d", memory->cells[i].value.types.i);
      }
      else if(value_type == RAM_TYPE_BOOLEAN){
        if(memory->cells[i].value.types.i == 0)
          printf("boolean, False");
        else
        {
          printf("boolean, True");      
        }
      }
      else if(value_type == RAM_TYPE_NONE){
        printf("none, None");
      }
   printf("\n");
  }

  printf("**END PRINT**\n");
}
