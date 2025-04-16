#!/bin/bash

# Create the output directory if it doesn't exist
mkdir -p my_outputs
echo "--------------------------"
echo "Beginning Round Robin Tests"
echo "--------------------------"


# Loop through test files numbered 2 to 15

for i in {1..15}; do
  # Execute the sched_rr program with the test file and redirect output
   ./sched_rr tests/test_rr_${i}.proc > my_outputs/test_rr_${i}.my

  # Compare the output with the expected output
  echo "Running test ${i}..."
  diff my_outputs/test_rr_${i}.my tests/test_rr_${i}.output
done

echo "--------------------------"
echo "END Round Robin Tests"
echo "--------------------------"