# Disable confirmation prompt for quitting
set confirm off

# Define the range of lines for the breakpoint
set $start_line = 179
set $end_line = 187

# Set a temporary breakpoint at the end_line
break $end_line
commands
    printf "\nReached end_line. Terminating program execution.\n"
    quit
end

# Loop from start_line to end_line
set $current_line = $start_line
while $current_line <= $end_line
    # Set the breakpoint at current_line
    break $current_line

    commands
        # Print stack information
        printf "\nStack Information:\n"
        p stack.top
        p stack.size

        # Print program counter
        printf "\nProgram Counter:\n"
        p program_counter

        # Print backtrace
        printf "\nBacktrace:\n"
        bt

        # Print source code
        printf "\nSource Code:\n"
        list $current_line


        # Continue execution
        continue
    end

    # Increment the current_line
    set $current_line = $current_line + 1
end

# Continue execution
continue
