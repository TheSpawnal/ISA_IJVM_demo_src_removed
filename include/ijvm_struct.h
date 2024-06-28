
#ifndef IJVM_STRUCT_H
#define IJVM_STRUCT_H

#include <stdio.h>  /* contains type FILE * */

#include "ijvm_types.h"
/**
 * All the state of your IJVM machine goes in this struct!
 **/

#define INITIAL_STACK_SIZE 256
#define MAX_LOCAL_VARIABLES 1024  
#define MAX_FRAMES 66000

typedef struct {
    word_t value;
    bool is_initialized;
} LocalVariable;

// stack frames sake 
typedef struct StackFrame {
    uint32_t saved_program_counter;
    uint32_t saved_frame_pointer;
    LocalVariable* local_variables;
    int local_variables_count;
    struct StackFrame* previous_frame;
} StackFrame;

typedef struct {
    word_t* elements;
    int top;
    int size;
} IJVMStack;

typedef struct IJVM {
    // do not changes these two variables
    FILE *in;   // use fgetc(ijvm->in) to get a character from in.
                // This will return EOF if no char is available.
    FILE *out;  // use for example fprintf(ijvm->out, "%c", value); to print value to out
  // your variables go here
    uint32_t text_size;
    uint32_t constant_pool_size;
    byte_t *text;
    uint32_t *constant_pool;
    int frames_stack_top;

    int frames_stack_size;

    uint32_t program_counter;
    int frame_pointer;
    bool halted;
    //stack & stack frames \2
    IJVMStack stack;
    StackFrame* frames_stack[MAX_FRAMES];
    //StackFrame** frames_stack;
} ijvm;

#endif 
