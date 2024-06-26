#include <stdio.h> 
#include <stdlib.h> 
#include "ijvm.h"
#include "util.h" 
#include <ctype.h>


void initialize_stack(IJVMStack *stack) {
    stack->elements = (word_t *)malloc(INITIAL_STACK_SIZE * sizeof(word_t));
    stack->top = -1;
    stack->size = INITIAL_STACK_SIZE;
}

void init_main_frame(ijvm* m){
    StackFrame* main_frame = (StackFrame *)malloc(sizeof(StackFrame));
    if (!main_frame) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    main_frame->local_variables = (LocalVariable *)malloc(MAX_LOCAL_VARIABLES * sizeof(LocalVariable));
    if (!main_frame->local_variables) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    main_frame->local_variables_count = MAX_LOCAL_VARIABLES;
    for (int i = 0; i < MAX_LOCAL_VARIABLES; i++) {
        main_frame->local_variables[i].value = 0;
        main_frame->local_variables[i].is_initialized = false;
    }
    main_frame->previous_frame = NULL;
    m->frames_stack[++m->frames_stack_top] = main_frame;

}

void push(IJVMStack *stack, word_t value) {
    if (stack->top == stack->size - 1) {
        int newSize = stack->size * 2;
        word_t *newElements = (word_t *)realloc(stack->elements, newSize * sizeof(word_t));
        if (newElements == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        stack->elements = newElements;
        stack->size = newSize;
    }
    stack->elements[++stack->top] = value;
}

word_t pop(IJVMStack *stack) {
    if (stack->top < 0) {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return stack->elements[stack->top--];
}

word_t get_local_variable(ijvm* m, int i) {
 //The value of the current instruction represented as a byte_t.
 //This should NOT increase the program counter.
    if(m->frames_stack_top < 0 || m->frames_stack_top >= MAX_FRAMES){
        fprintf(m->out, "Error get_local_variable : stack frame pointer out of bounds \n");
        exit(EXIT_FAILURE);
    }
    StackFrame* current_frame = m->frames_stack[m->frames_stack_top];
    if (i < 0 || i >= current_frame->local_variables_count) {
        fprintf(stderr, "Error: Local variable index %d out of bounds. Max index is %d.\n", i, current_frame->local_variables_count - 1);
        exit(EXIT_FAILURE);} // or return a special error code if you prefer
    if(!current_frame->local_variables[i].is_initialized){
        fprintf(stderr, "Error: Local variable at index %d not initialized.\n", i);
        exit(EXIT_FAILURE);}
    return current_frame->local_variables[i].value;
}

bool set_local_variable(ijvm* m, int i, word_t value) {
    if(m->frames_stack_top < 0 || m->frames_stack_top >= MAX_FRAMES){
        fprintf(m->out, "Error set_local_variable : stack frame pointer out of bounds \n");
        exit(EXIT_FAILURE);
    }
    StackFrame* current_frame = m->frames_stack[m->frames_stack_top];
    if (i < 0 || i >= current_frame->local_variables_count) {
        fprintf(stderr, "Error: Local variable index %d out of bounds. Max index is %d.\n", i, current_frame->local_variables_count - 1);
        return false;
    }
    current_frame->local_variables[i].value = value;
    current_frame->local_variables[i].is_initialized = true;
    return true;
}

uint16_t fetch_next_short(ijvm* m){
    uint16_t result = 0;
    uint8_t numbuf[2];
    numbuf[0] = m->text[m->program_counter + 1];
    numbuf[1] = m->text[m->program_counter + 2];
    result = read_uint16(numbuf);
    m->program_counter += 2;
    return result;
}

//word_t fetch_next_byte(ijvm* m){
uint8_t fetch_next_byte(ijvm* m){
    if(m->program_counter + 1 >= m->text_size){
        fprintf(stderr, "Error: program_counter exceeds text size\n");
        exit(EXIT_FAILURE);
    }
    d4printf("fetch_next_byte: program_counter = %d\n", m->program_counter);
    d4printf("fetch_next_byte: text_size = %d\n", m->text_size);
    d4printf("fetch_next_byte: text[program_counter] = %d\n", m->text[m->program_counter]);
    uint8_t result_fetch_next_bytes = m->text[m->program_counter+1];
    m->program_counter++;
    d4printf("fetch_next_byte: result_fetch_next_bytes = %d\n", result_fetch_next_bytes);
    return result_fetch_next_bytes;
}

void ensure_local_variables_space(StackFrame* frame, int required_index) {
    if (required_index >= frame->local_variables_count) {
        int new_size = required_index + 1; // Or more, depending on your resizing strategy
        frame->local_variables = realloc(frame->local_variables, new_size * sizeof(LocalVariable));
        if (!frame->local_variables) {
            fprintf(stderr, "Error: failed to resize local variables\n");
            exit(EXIT_FAILURE);
        }
        // Initialize the new elements
        for (int i = frame->local_variables_count; i < new_size; i++) {
            frame->local_variables[i].value = 0;
            frame->local_variables[i].is_initialized = false;
        }
        frame->local_variables_count = new_size;
    }
}
void handle_invokevirtual(uint16_t index,ijvm* m){
    //fetch the method adress from the constant pool
    uint32_t method_adress = get_constant(m, index);
    uint16_t arg_count = read_uint16(&m->text[method_adress]);
    uint16_t local_var_size= read_uint16(&m->text[method_adress + 2]);
    uint16_t code_start  = method_adress + 4; 

    //create a new stack frame for the method call
    StackFrame* new_frame = (StackFrame *)malloc(sizeof(StackFrame));
    if(!new_frame){
        fprintf(stderr, "Error: failed to allocate memory for new frame\n");
        exit(EXIT_FAILURE);
    }
    new_frame->local_variables = (LocalVariable *)malloc(local_var_size * sizeof(LocalVariable));
    if(!new_frame->local_variables){
        fprintf(stderr, "Error: failed to allocate memory for local variables\n");
        exit(EXIT_FAILURE);
    }
    new_frame->local_variables_count = local_var_size;
    for (int i = 0 ; i < local_var_size; i++) {
        new_frame->local_variables[i].value = 0;
        new_frame->local_variables[i].is_initialized = false;
    }
    //transfert arguments from the stack to the new frame
    for (int i = arg_count; i > 0; i--) {
        word_t value = pop(&m->stack);
        new_frame->local_variables[i].value = value;
        new_frame->local_variables[i].is_initialized = true;
    }   
    //save the caller's state in the new frame
    new_frame->previous_frame = m->frames_stack[m->frames_stack_top];
    new_frame->saved_program_counter = m->program_counter + 3; // Adjust PC after the whole instruction
    new_frame->saved_frame_pointer = m->frame_pointer;
    m->frames_stack[++m->frames_stack_top] = new_frame;
    m->frame_pointer = m->frames_stack_top;
    m->program_counter = code_start;
}

void handle_ireturn(ijvm* m){
    //get the current frame
    StackFrame* current_frame = m->frames_stack[m->frames_stack_top];
    //retrieve the return value from the top of the current stack frame
    word_t return_value = pop(&m->stack);
    //restore the caller's state (local variables, pc, stack pter)
    m->frames_stack_top--;
    StackFrame* previous_frame = m->frames_stack[m->frames_stack_top];
    m->program_counter = current_frame->saved_program_counter;
    m->frame_pointer = current_frame->saved_frame_pointer;
    //place the return value on the top of the callers stack 
    push(&m->stack, return_value);
    //free the current frame
    free(current_frame->local_variables);
    free(current_frame);
}
void handle_GOTO(ijvm *m) {
    d3printf("Stack size before OP_GOTO: %d\n", m->stack.top);
    if (m->program_counter + 2 >= m->text_size || m->program_counter < 0) {
    //if (m->program_counter + 2 >= m->text_size) {
        fprintf(stderr, "Error: program_counter exceeds text size\n");
        exit(EXIT_FAILURE);
    }
    //signed short offset = read_uint16(&m->text[m->program_counter + 1]);
    signed short offset = (signed short)read_uint16(&m->text[m->program_counter + 1]);
    d3printf("GOTO: Program counter before calculation: %d, offset: %d\n", m->program_counter, offset);
    m->program_counter += offset;
    m->program_counter -= 1;
    d3printf("GOTO: Program counter after calculation: %d\n", m->program_counter);
    d3printf("Stack size after OP_GOTO: %d\n", m->stack.top);
}

void handle_IFEQ(ijvm *m) {
    d3printf("Stack size before OP_IFEQ: %d\n", m->stack.top );
    //if (m->program_counter + 2 >= m->text_size || m->program_counter < 0) {
    if (m->program_counter + 2 >= m->text_size ) {
        fprintf(stderr, "Error: program_counter exceeds text size\n");
        exit(EXIT_FAILURE);
    }
    signed short offset = read_int16(&m->text[m->program_counter + 1]);
    word_t value = pop(&m->stack);
    d3printf("IFEQ: Value popped: %d, offset: %d\n", value, offset);
    if (value == 0) {
        m->program_counter += offset;
    } else {
        m->program_counter += 3;
    }
    m->program_counter -= 1;
    d3printf("IFEQ: Program counter after evaluation: %d\n", m->program_counter);
}

void handle_IFLT(ijvm *m) {
    d3printf("Stack size before OP_IFLT: %d\n", m->stack.top);
    if (m->program_counter + 2 >= m->text_size || m->program_counter < 0) {
        fprintf(stderr, "Error: program_counter exceeds text size\n");
        exit(EXIT_FAILURE);
    }
    signed short offset = read_int16(&m->text[m->program_counter + 1]);
    word_t value = pop(&m->stack);
    d3printf("IFLT: Value popped: %d, offset: %d\n", value, offset);
    if (value < 0) {
        m->program_counter += offset;
    } else {
        m->program_counter += 3;
    }
    m->program_counter -= 1;
    d3printf("IFLT: Program counter after evaluation: %d\n", m->program_counter);
}

void handle_IF_ICMPEQ(ijvm *m) {
    d3printf("Stack size before OP_IF_ICMPEQ: %d\n", m->stack.top );
    if (m->program_counter + 2 >= m->text_size) {
        fprintf(stderr, "Error: program_counter exceeds text size\n");
        exit(EXIT_FAILURE);
    }
    signed short offset = read_int16(&m->text[m->program_counter + 1]);
    word_t value1 = pop(&m->stack);
    word_t value2 = pop(&m->stack);
    d3printf("IF_ICMPEQ: Values popped: %d, %d, offset: %d\n", value1, value2, offset);
    if (value1 == value2) {
        m->program_counter += offset;
    } else {
        m->program_counter += 3;
    }
    m->program_counter -= 1;
    d3printf("IF_ICMPEQ: Program counter after evaluation: %d\n", m->program_counter);
}

void handle_op_ldc_w( uint16_t index, ijvm* m) {
    d4printf("Stack size before OP_LDC_W: %d\n", m->stack.top);
    word_t value = m->constant_pool[index];
    push(&m->stack, value);
    d4printf("Stack size after OP_LDC_W: %d\n", m->stack.top);
}

void handle_op_istore(uint16_t index, ijvm* m) {
    if (m->frames_stack_top < 0 || m->frames_stack_top >= MAX_FRAMES) {
        fprintf(stderr, "Invalid frame stack top index\n");
        exit(EXIT_FAILURE);
    }

    d4printf("Stack size before OP_IStore: %d\n", m->stack.top);
    d4printf("Attempting to store to local variable at index %d\n", index);

    StackFrame* currentFrame = m->frames_stack[m->frames_stack_top];
    ensure_local_variables_space(currentFrame, index);

    if (index >= currentFrame->local_variables_count) {
        fprintf(stderr, "Local variable index %d out of bounds\n", index);
        exit(EXIT_FAILURE);
    }

    if (m->stack.top < 0) {
        fprintf(stderr, "Stack underflow error\n");
        exit(EXIT_FAILURE);
    }

    word_t value = pop(&m->stack);
    currentFrame->local_variables[index].value = value;
    currentFrame->local_variables[index].is_initialized = true;

    d4printf("Stored value %x to local variable at index %d\n", value, index);
    d4printf("Stack size after OP_ISTORE: %d\n", m->stack.top);
}

void handle_op_iload(uint16_t index, ijvm* m) {
    if (m->frames_stack_top < 0 || m->frames_stack_top >= MAX_FRAMES) {
        fprintf(stderr, "Invalid frame stack top index\n");
        exit(EXIT_FAILURE);
    }
    d4printf("Stack size before OP_ILOAD: %d\n", m->stack.top);
    ensure_local_variables_space(m->frames_stack[m->frames_stack_top], index);
    StackFrame* currentFrame = m->frames_stack[m->frames_stack_top];
    if (index >= currentFrame->local_variables_count || !currentFrame->local_variables[index].is_initialized) {
        fprintf(stderr, "Uninitialized or out-of-bounds local variable access at index %d\n", index);
        exit(EXIT_FAILURE);
    }
    word_t value = currentFrame->local_variables[index].value;
    push(&m->stack, value);
    d4printf("Stack size after OP_ILOAD: %d\n", m->stack.top);
}

void handle_op_iinc(ijvm* m, uint16_t index, int8_t increment){
    d4printf("Stack size before OP_IINC: %d\n", m->stack.top);
    d4printf("Attempting to increment local variable at index %d by %d\n", index, increment);
    StackFrame* currentFrame = m->frames_stack[m->frames_stack_top];
    ensure_local_variables_space(currentFrame, index + 1);
    word_t value = get_local_variable(m, index);
    value += increment;
    set_local_variable(m, index, value);
    d4printf("Incremented local variable at index %d to %d\n", index, value);
    d4printf("Stack size after OP_IINC: %d\n", m->stack.top);
}

ijvm* init_ijvm(char *binary_path, FILE* input, FILE* output) {
    ijvm* m = (ijvm *)malloc(sizeof(ijvm));
    if (!m) {
        return NULL;
    }

    FILE *file = fopen(binary_path, "rb");
    if (!file) {
        free(m);
        return NULL;
    }
    m->in = input;
    m->out = output;
    m->halted = false;
    m->frames_stack_top = -1;
    m->program_counter = 0;
    m->frame_pointer = 0;

    uint8_t numbuf[4];
    fread(numbuf, sizeof(uint8_t), 4, file);
    uint32_t magic_number = read_uint32(numbuf);
    if (magic_number != MAGIC_NUMBER) {
        fclose(file);
        free(m);
        return NULL;
    }

    fread(numbuf, sizeof(uint8_t), 4, file);
    fread(numbuf, sizeof(uint8_t), 4, file);
    m->constant_pool_size = read_uint32(numbuf) / sizeof(uint32_t);
    m->constant_pool = malloc(m->constant_pool_size * sizeof(uint32_t));
    if (!m->constant_pool) {
        fclose(file);
        free(m);
        return NULL;
    }

    for (unsigned int i = 0; i < m->constant_pool_size; i++) {
        fread(numbuf, sizeof(uint8_t), 4, file);
        m->constant_pool[i] = read_uint32(numbuf);
    }

    fread(numbuf, sizeof(uint8_t),4, file);
    fread(numbuf, sizeof(uint8_t),4, file);
    m->text_size = read_uint32(numbuf);
    m->text = malloc(m->text_size * sizeof(byte_t));
    if (!m->text) {
        fclose(file);
        free(m->constant_pool);
        free(m);
        return NULL;
    }
    fread(m->text, sizeof(byte_t), m->text_size, file);
    fclose(file);

    initialize_stack(&m->stack);
    init_main_frame(m); // Initialize the main method frame

    return m;
}

void destroy_ijvm(ijvm* m) {
    free(m->text);
    free(m->constant_pool);
    free(m->stack.elements) ;
    free(m);
}

byte_t *get_text(ijvm* m) {
    return m ? m->text : NULL;
}

unsigned int get_text_size(ijvm* m) {
    return m ? m->text_size:0;
}

word_t get_constant(ijvm* m, int i) {
    if (m && i >= 0 && i < m->constant_pool_size) {
        return m->constant_pool[i];
    }
    fprintf(m->out, "Error: constant index out of bounds\n");
    return 0;
}

unsigned int get_program_counter(ijvm* m) {
    return m->program_counter;
}

word_t tos(ijvm* m) {
    if (m->stack.top < 0) {
        fprintf(m->out, "shtack empty\n");
        exit(EXIT_FAILURE);
    }
    return m->stack.elements[m->stack.top];
}

bool finished(ijvm* m) {
    return (m->program_counter >= m->text_size) || (m->halted);
}

void step(ijvm* m) {
    if (finished(m)) {
        d3printf("Program termination\n");
        return;
    }
    
    byte_t opcode = get_instruction(m);
    d3printf("Current instruction: %x\n", opcode);

    switch (opcode) {
        case OP_BIPUSH: {
            int8_t value = (int8_t)m->text[++m->program_counter];
            push(&m->stack, value);
            break;
        }
        case OP_DUP: {
            word_t value = tos(m);
            push(&m->stack, value);
            break;
        }
        case OP_IADD: {
            word_t value1 = pop(&m->stack);
            word_t value2 = pop(&m->stack);
            push(&m->stack, value1 + value2);
            break;
        }
        case OP_IAND: {
            word_t value1 = pop(&m->stack);
            word_t value2 = pop(&m->stack);
            push(&m->stack, value1 & value2);
            break;
        }
        case OP_IOR: {
            word_t value1 = pop(&m->stack);
            word_t value2 = pop(&m->stack);
            push(&m->stack, value1 | value2);
            break;
        }
        case OP_ISUB: {
            word_t value1 = pop(&m->stack);
            word_t value2 = pop(&m->stack);
            push(&m->stack, value2 - value1);
            break;
        }
        case OP_NOP:
            break;
        case OP_POP:
            pop(&m->stack);
            break;
        case OP_SWAP: {
            word_t value1 = pop(&m->stack);
            word_t value2 = pop(&m->stack);
            push(&m->stack, value1);
            push(&m->stack, value2);
            break;
        }
        case OP_ERR:
            fprintf(m->out, "IJVM error\n");
            m->halted = true;
            break;
        case OP_HALT:
            m->halted = true;  
            break;
        case OP_OUT: {
            word_t value = pop(&m->stack);
            fprintf(m->out, "%c", (char)value);
            break;
        }

        case OP_IN: {
            int value = fgetc(m->in);
            push(&m->stack, (word_t)(value == EOF ? 0 : value));
            break;
        }
        case OP_GOTO:
            handle_GOTO(m);
            break;
        case OP_IFEQ:
            handle_IFEQ(m);
            break;
        case OP_IFLT:
            handle_IFLT(m);
            break;
        case OP_IF_ICMPEQ:
            handle_IF_ICMPEQ(m);
            break;

        case OP_LDC_W:{
            uint16_t index = fetch_next_short(m);
            handle_op_ldc_w(index,m);
            break;
        }
        case OP_ISTORE:{
            uint16_t index = fetch_next_byte(m);
            handle_op_istore(index, m);
            break;
        }
        case OP_ILOAD:{
            uint16_t index = fetch_next_byte(m);
            handle_op_iload(index, m);
            break;
        }
        case OP_IINC:{
        uint16_t index = fetch_next_byte(m);
        int8_t increment_value = (int8_t) fetch_next_byte(m);
        handle_op_iinc(m, index, increment_value);
        break;
        }
        case OP_WIDE: {
            uint8_t next_opcode = fetch_next_byte(m);
            switch(next_opcode) {
                case OP_ILOAD: {
                    uint16_t index = fetch_next_short(m);
                    handle_op_iload(index, m);
                    break;
                }
                case OP_ISTORE: {
                    uint16_t index = fetch_next_short(m);
                    handle_op_istore(index, m);
                    break;
                }
                case OP_IINC: {
                    uint16_t index = fetch_next_short(m);
                    int8_t increment_value = (int8_t)fetch_next_byte(m);
                    handle_op_iinc(m, index, increment_value);
                    break;
                }
                default:
                    fprintf(m->out, "Error: Invalid instruction after WIDE\n");
                    m->halted = true;
            }
            break;
        }
        case OP_INVOKEVIRTUAL:{
            uint16_t index = fetch_next_short(m);
            handle_invokevirtual(index,m);
            break;
        }
        case OP_IRETURN:{
            handle_ireturn(m);
            break;
        }
        default:
            fprintf(m->out, "Unknown instruction: 0x%02x\n", opcode);
            m->halted = true;
            break;
    }
    m->program_counter++;
}

byte_t get_instruction(ijvm* m) { 
  return get_text(m)[get_program_counter(m)]; 
}

ijvm* init_ijvm_std(char *binary_path){
  return init_ijvm(binary_path, stdin, stdout);
}

void run(ijvm* m) {
  while (!finished(m)){
    step(m);
  }
}

// Below: methods needed by bonus tests, see ijvm.h
// You can leave these unimplemented if you are not doing these bonus 

int get_call_stack_size(ijvm* m) {
   // TODO: implement me if doing tail call bonus
   return 0;
}


// Checks if reference is a freed heap array. Note that this assumes that  
bool is_heap_freed(ijvm* m, word_t reference) {
   // TODO: implement me if doing garbage collection bonus
   return 0;
}
