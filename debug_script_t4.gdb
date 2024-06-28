# Disable confirmation prompt for quitting
set confirm off

# Define the range of lines for the breakpoint
set $start_line = 23
set $end_line = 27

# Set a temporary breakpoint at the end_line
break $end_line
commands
    printf "\nReached end_line. Terminating program execution.\n"
    quit
end

# Define a function to print stack information
define print_stack_info
    printf "\nStack Information:\n"
    printf "Stack size: %d\n", stack.size
    printf "Top: %d\n", stack.top
end

# Define a function to print program counter
define print_program_counter
    printf "\nProgram Counter:\n"
    printf "Program_counter: %d\n", program_counter
end

# Define a function to print variables
define print_variables
    printf "\nVariables:\n"
    printf "index: %d\n", index
    # Add additional variables here
end

# Define a function to print frame information
define print_frame_info
    printf "\nFrame Information:\n"
    # Add frame-related information here
end

# Define a function to print backtrace
define print_backtrace
    printf "\nBacktrace:\n"
    bt
end

# Define a function to print source code
define print_source_code
    printf "\nSource Code:\n"
    list $current_line
end

# Define a function to print the value of a pointer variable
define print_pointer_value
    printf "\nPointer Value:\n"
    printf "*pointer_variable: %d\n", *pointer_variable
end

# Define a function to print memory content at a specific address
define print_memory_content
    printf "\nMemory Content:\n"
    x/xg $address
end

# Loop from start_line to end_line
set $current_line = $start_line
while $current_line <= $end_line
    # Set the breakpoint at current_line
    break $current_line

    commands
        # Print debug information
        print_stack_info
        print_program_counter
        print_variables
        print_frame_info
        print_backtrace
        print_source_code

        # Print pointer value if applicable
        if $current_line == 12
            print_pointer_value
        end

        # Print memory content if applicable
        if $current_line == 39
            print_memory_content
        end

        # Continue execution
        continue
    end

    # Increment the current_line
    set $current_line = $current_line + 1
end

# Continue execution
continue