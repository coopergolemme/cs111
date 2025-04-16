#!/bin/bash
 
mkdir -p my_outputs

for test_file in ./my_tests/*; do
  # Extract the base name of the test file (e.g., test_rr_1.proc)


  test_name=$(basename "$test_file")
    echo "Running test: $test_name"
    valgrind ./vmm "$test_file" 

done 