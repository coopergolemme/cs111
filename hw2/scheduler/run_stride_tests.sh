#!/bin/bash

# Create the output directory if it doesn't exist
mkdir -p my_outputs
make 
echo "--------------------------"
echo "Beginning Stride Tests"
echo "--------------------------"

# Loop through test files numbered 2 to 15
for i in {1..15}; do
  # Execute the sched_rr program with the test file and redirect output
  ./sched_stride tests/test_stride_${i}.proc > my_outputs/test_stride_${i}.my

  # Compare the output with the expected output
    echo "Running test ${i}..."
  diff my_outputs/test_stride_${i}.my tests/test_stride_${i}.output
done
echo "--------------------------"
echo "End Stide Tests"
echo "--------------------------"