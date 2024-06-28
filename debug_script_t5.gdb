 #Set a breakpoint at the start of the test_invoke2 function
break test_invoke2
commands
  echo [INFO] Starting test_invoke2...\n
  continue
end

# Set breakpoints at the start and end of OP_INVOKEVIRTUAL and OP_IRETURN operations
break 51
break 52
break 56
break 61

# When we hit the start of OP_INVOKEVIRTUAL
commands
  echo [INFO] Entered OP_INVOKEVIRTUAL...\n
  
  # Print machine state
  print program_counter
  print machine
  
  # Print current stack state
  print stack.top
  print stack.size
  echo Stack elements: \n
  # Assuming the stack has a function to iterate through elements
  # Iterate over the stack and print elements
  # Replace 'iterate_stack' with an appropriate method if available
  call iterate_stack(stack)

  continue
end

# When we exit OP_INVOKEVIRTUAL
commands
  echo [INFO] Exiting OP_INVOKEVIRTUAL...\n
  
  # Print machine state again to spot any changes
  print program_counter
  print machine
  
  # Print current stack state
  print stack.top
  print stack.size

  continue
end

# When we hit the start of OP_IRETURN
commands
  echo [INFO] Entered OP_IRETURN...\n
  
  # Print machine state
  print program_counter
  print machine
  
  # Print current stack state
  print stack.top
  print stack.size

  continue
end

# When we exit OP_IRETURN
commands
  echo [INFO] Exiting OP_IRETURN...\n
  
  # Print machine state to spot any changes
  print program_counter
  print machine
  
  # Print current stack state
  print stack.top
  print stack.size

  continue
end

# Start the program execution
run

# End of the GDB script