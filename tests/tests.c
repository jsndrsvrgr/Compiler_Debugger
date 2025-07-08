/*tests.c*/

//
// << DESCRIPTION >>
//
// << YOUR NAME >>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ram.h"
#include "gtest/gtest.h"

//
// private helper functions:
//


//
// some provided unit tests to get started:
//
TEST(memory_module, initialization)
{
  //
  // create a new memory and make sure it's initialized properly:
  //
  struct RAM* memory = ram_init();

  ASSERT_TRUE(memory != NULL);        // use ASSERT_TRUE with pointers
  ASSERT_TRUE(memory->cells != NULL);

  ASSERT_EQ(memory->num_values, 0);  // use ASSERT_EQ for comparing values
  ASSERT_EQ(memory->capacity, 4);

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, read_by_name_does_not_exist) 
{
  //
  // create a new memory:
  //
  struct RAM* memory = ram_init();

  //
  // read a var that doesn't exist:
  //
  struct RAM_VALUE* value = ram_read_cell_by_name(memory, "x");
  ASSERT_TRUE(value == NULL);  // use ASSERT_TRUE with pointers

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, write_one_int) 
{
  //
  // create a new memory:
  //
  struct RAM* memory = ram_init();

  //
  // store the integer 123:
  //
  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_INT;
  i.types.i = 123;

  bool success = ram_write_cell_by_name(memory, i, "x");
  ASSERT_TRUE(success);

  //
  // now check the memory, was x = 123 stored properly?
  //
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->cells[0].value.value_type, RAM_TYPE_INT);
  ASSERT_EQ(memory->cells[0].value.types.i, 123);
  ASSERT_STREQ(memory->cells[0].identifier, "x");  // strings => ASSERT_STREQ

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, read_and_write_name_addr)
{
  //
  //Create a new memory:

  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  value.types.i = 55;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "y"));
  int address = ram_get_addr(memory, "y");
  ASSERT_NE(address, -1);
  ASSERT_EQ(address, 0);
  struct RAM_VALUE* value_read =  ram_read_cell_by_name(memory, "y");
  ASSERT_TRUE(value_read != NULL);
  ASSERT_EQ(value_read->value_type, RAM_TYPE_INT);
  ASSERT_EQ(value_read->types.i, 55);
  ram_free_value(value_read);

  struct RAM_VALUE second_value;
  second_value.value_type = RAM_TYPE_STR;
  second_value.types.s = "cat";
  ASSERT_TRUE(ram_write_cell_by_name(memory, second_value, "x"));

  value_read = ram_read_cell_by_addr(memory, 1);
  ASSERT_EQ(value_read->value_type, RAM_TYPE_STR);
  ASSERT_STREQ(value_read->types.s, "cat");
  ram_free_value(value_read);

  second_value.types.s = "home";
  ASSERT_TRUE(ram_write_cell_by_addr(memory, second_value, 1));
  value_read = ram_read_cell_by_addr(memory, 1);
  ASSERT_STREQ(value_read->types.s, "home");
  ram_free_value(value_read);
  int address_2 = ram_get_addr(memory, "x");
  ASSERT_EQ(address_2, 1);

  ASSERT_FALSE(ram_write_cell_by_addr(memory, second_value, 2));
  ASSERT_FALSE(ram_write_cell_by_addr(memory, second_value, -1));

  ram_destroy(memory);
}


TEST(memory_module, string_not_duplicated){
  struct RAM* memory = ram_init();
  char string[10] = "cat";

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_STR;
  value.types.s = string;

  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "x"));

  string[2] = 'r'; //cat becomes car

  struct RAM_VALUE* read_value = ram_read_cell_by_name(memory, "x");
  ASSERT_STREQ(read_value->types.s, "cat");
  ASSERT_NE(read_value->types.s, string);
  ram_free_value(read_value);
  ram_destroy(memory);
}

TEST(memory_module, doubling_string) {
    struct RAM* memory = ram_init();

    ASSERT_EQ(memory->capacity, 4);

    char name[2] = "A";

    for (int i = 1; i <= 5; i++) {
        struct RAM_VALUE val;
        val.value_type = RAM_TYPE_INT;
        val.types.i = i;
        ASSERT_TRUE(ram_write_cell_by_name(memory, val, name));
        name[0]++;
    }

    ASSERT_EQ(memory->capacity, 8);

    name[0] = 'F'; // Continue 
    for (int i = 0; i <= 4; i++) {
        struct RAM_VALUE val;
        val.value_type = RAM_TYPE_INT;
        val.types.i = i;
        ASSERT_TRUE(ram_write_cell_by_name(memory, val, name));
        name[0]++;
    }
    // Now there are 9 variables; capacity should have doubled from 8 to 16.
    ASSERT_EQ(memory->capacity, 16);

    ram_destroy(memory);
}
TEST(memory_module, initialize_new_array) {
    struct RAM* memory = ram_init();

    char name[2] = "A";
    for (int i = 1; i <= 4; i++) {
        struct RAM_VALUE val;
        val.value_type = RAM_TYPE_INT;
        val.types.i = i;
        ASSERT_TRUE(ram_write_cell_by_name(memory, val, name));
        name[0]++;
    }

    for (int i = 4; i < memory->capacity; i++) {
        ASSERT_TRUE(memory->cells[i].identifier == NULL);
        ASSERT_EQ(memory->cells[i].value.value_type, RAM_TYPE_NONE);
    }

    ram_destroy(memory);
}