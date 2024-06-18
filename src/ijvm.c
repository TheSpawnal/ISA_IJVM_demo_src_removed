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

void push(IJVMStack *stack, word_t value) {
    if (stack->top == stack->size - 1) {
        stack->size *= 2;
        stack->elements = (word_t *)realloc(stack->elements, stack->size * sizeof(word_t));
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

word_t get_local_variable(ijvm* m, int i) {
  // TODO: implement me
  return 0;
}

void step(ijvm* m) {
    byte_t opcode = get_instruction(m);

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
