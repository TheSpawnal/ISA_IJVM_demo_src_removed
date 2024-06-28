
ijvm* init_ijvm(char *binary_path, FILE* input, FILE* output) {
    //dprintf("Initializing IJVM...\n");
    ijvm* m = (ijvm *)malloc(sizeof(ijvm));
    if (!m) {
        fprintf(stderr, "Failed to allocate memory for ijvm struct\n");
        return NULL;
    }
    FILE *file = fopen(binary_path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file %s\n", binary_path);
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
    //word_t numbuf[4];
    fread(numbuf, sizeof(uint8_t), 4, file);
    uint32_t magic_number = read_uint32(numbuf);
    //dprintf("Magic number: %x\n", magic_number);
    if (magic_number != MAGIC_NUMBER) {
        dprintf("Invalid magic number : %x\n", magic_number);
        //dprintf("Invalid magic number : %u\n", magic_number);
        fclose(file);
        free(m);
        return NULL;
    }
    fread(numbuf, sizeof(uint8_t), 4, file);
    fread(numbuf, sizeof(uint8_t), 4, file);

    m->constant_pool_size = read_uint32(numbuf) / sizeof(uint32_t);
    m->constant_pool = malloc(m->constant_pool_size * sizeof(uint32_t));
    if (!m->constant_pool) {
        fprintf(stderr, "Failed to allocate memory for constant pool\n");
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
        fprintf(stderr, "Failed to allocate memory for text sectino\n");
        fclose(file);
        free(m->constant_pool);
        free(m);
        return NULL;
    }
    fread(m->text, sizeof(byte_t), m->text_size, file);
    //dprintf("Initialized machine with text size (number of instructions): %d\n", m->text_size);
    fclose(file);
    // dprintf("init_ijvm: m->text.size = %d\n", m->text_size);
    // dprintf("init_ijvm: m->constant_pool.size = %d\n", m->constant_pool_size);
    // dprintf("init_ijvm: m->text:\n");
    // for (int i = 0; i < m->text_size; i++) {
    //     dprintf("%x ", m->text[i]);
    // }
    // dprintf("\n");
    initialize_stack(&m->stack);
    //fprintf(stderr, "IJVM initialized with size %d\n", INITIAL_STACK_SIZE);
    init_main_frame(m); 
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
    return m ? m->text_size : 0;
}

word_t get_constant(ijvm* m, int i) {
    if (m && i >= 0 && i < m->constant_pool_size) {
        return m->constant_pool[i];
    }
    fprintf(m->out, "Error: constant index out of bounds\n");
    return 0;
}