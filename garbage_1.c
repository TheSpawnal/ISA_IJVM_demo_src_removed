void initialize_stack(IJVMStack *stack) {
    stack->elements = (word_t *)malloc(INITIAL_STACK_SIZE * sizeof(word_t));
    stack->top = -1;
    stack->size = INITIAL_STACK_SIZE;
}
StackFrame* initialize_frame(ijvm* m, int num_locals, bool is_main) {
    if (!is_main && m->frames_stack_top >= MAX_FRAMES - 1) {
        fprintf(stderr, "Error: frame stack overflow\n");
        exit(EXIT_FAILURE);
    }
    StackFrame* frame = (StackFrame *)malloc(sizeof(StackFrame));
    if (!frame) {
        fprintf(stderr, "Failed to allocate memory for frame\n");
        exit(EXIT_FAILURE);
    }
    int total_vars = is_main ? MAX_LOCAL_VARIABLES : num_locals;
    frame->local_variables = (LocalVariable *)malloc(total_vars * sizeof(LocalVariable));
    if (!frame->local_variables) {
        fprintf(stderr, "Failed to allocate memory for local variables\n");
        exit(EXIT_FAILURE);
    }
    frame->local_variables_count = total_vars;
    for (int i = 0; i < total_vars; i++) {
        frame->local_variables[i].value = 0;
        frame->local_variables[i].is_initialized = false;
    }
    frame->saved_program_counter = 0;  
    frame->saved_frame_pointer = 0;    
    frame->previous_frame = m->frames_stack_top >= 0 ? m->frames_stack[m->frames_stack_top] : NULL;
    if (is_main) {
        m->frames_stack_top = 0; 
    } else {
        m->frames_stack[++m->frames_stack_top] = frame; 
    }
    return frame;
}
void init_main_frame(ijvm* m) {
    StackFrame* main_frame = initialize_frame(m, 0, true);
    m->frames_stack[0] = main_frame;
}
StackFrame* create_frame(int num_args, int num_locals, ijvm* m) {
    int total_vars = num_args + num_locals;
    return initialize_frame(m, total_vars, false);
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
    if(m->frames_stack_top < 0 || m->frames_stack_top >= MAX_FRAMES){
        fprintf(m->out, "Error get_local_variable : stack frame pointer out of bounds \n");
        exit(EXIT_FAILURE);
    }
    StackFrame* current_frame = m->frames_stack[m->frames_stack_top];
    if (i < 0 || i >= current_frame->local_variables_count) {
        fprintf(stderr, "Error: Local variable index %d out of bounds. Max index is %d.\n", i, current_frame->local_variables_count - 1);
        exit(EXIT_FAILURE);} 
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
uint8_t fetch_next_byte(ijvm* m){
    if(m->program_counter + 1 >= m->text_size){
        fprintf(stderr, "Error: program_counter exceeds text size\n");
        exit(EXIT_FAILURE);
    }
    uint8_t result_fetch_next_bytes = m->text[m->program_counter+1];
    m->program_counter++;
    return result_fetch_next_bytes;
}
void ensure_local_variables_space(StackFrame* frame, int required_index) {
    if (required_index >= frame->local_variables_count) {
        int new_size = required_index + 1; 
        frame->local_variables = realloc(frame->local_variables, new_size * sizeof(LocalVariable));
        if (!frame->local_variables) {
            fprintf(stderr, "Error: failed to resize local variables\n");
            exit(EXIT_FAILURE);
        }
        for (int i = frame->local_variables_count; i < new_size; i++) {
            frame->local_variables[i].value = 0;
            frame->local_variables[i].is_initialized = false;
        }
        frame->local_variables_count = new_size;
    }
}
void handle_invokevirtual(uint16_t index, ijvm* m){
    uint32_t method_address = get_constant(m, index);
    uint16_t num_args = read_uint16(&m->text[method_address]);
    uint16_t num_locals = read_uint16(&m->text[method_address + 2]);
    StackFrame* new_frame = create_frame(num_args, num_locals, m);
    for(int i = num_args - 1; i >= 0; i--){
        word_t arg = pop(&m->stack);
        set_local_variable(m, i, arg);
    }
    uint16_t saved_program_counter = m->program_counter;
    m->program_counter = method_address + 3;
    new_frame->saved_program_counter = saved_program_counter;
    new_frame->saved_frame_pointer = m->frame_pointer;
    m->frame_pointer = m->frames_stack_top;
}
void handle_ireturn(ijvm* m){
    if(m->frames_stack_top < 0){
        fprintf(stderr, "Error IRETRUN: No active frames to return from.\n");
        exit(EXIT_FAILURE);
    }
    word_t return_value = pop(&m->stack);
    StackFrame* current_frame = m->frames_stack[m->frames_stack_top];
    m->program_counter = current_frame->saved_program_counter;
    m->frame_pointer = current_frame->saved_frame_pointer;
    free(current_frame->local_variables);
    free(current_frame);
    m->frames_stack[m->frames_stack_top] = NULL;
    m->frames_stack_top--;
    push(&m->stack, return_value);
}