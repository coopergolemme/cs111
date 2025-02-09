#!/bin/bash
make

echo -e "\n-----------------------\n|Running test cases...|\n-----------------------\n\n"

# Function to run a test case
run_test() {
    local input="$1"
    local expected_output="$2"
    local test_output

    # Run the shell program with the input and capture both stdout and stderr
    test_output=$(echo -e "$input" | ./shell 2>&1)

    # Compare the output with the expected output
    if [ "$test_output" == "$expected_output" ]; then
        echo "Test passed"
    else
        echo "!!!! Test failed!!!!"
        echo "Input: $input"
        echo "Expected: $expected_output"
        echo "Got: $test_output"
    fi
}

# Define test cases
test_cases=(
    "exit"
    "ls -l\nexit"
    "ls -l\nwc lillethd-oregon.pem\nexit"
)

# Define expected outputs
expected_outputs=(
    "jsh$ "
    "$(ls -l)
jsh$ jsh status: 0
jsh$ "
    "$(ls -l)"$'\n'"jsh$ wc lillethd-oregon.pem"$'\n'"jsh status: 0"$'\n'"jsh$ "

    # "jsh$ $(ls -l)\njsh status: 0\njsh$ "
    # "jsh$ $(wc lillethd-oregon.pem)\njsh status: 0\njsh$ "
    # "jsh$ ls: cannot access filedoesnotexist: No such file or directory\njsh status: 2\njsh$ "
    # "jsh$ jsh error: Command not found: foo\njsh$ jsh status: 127\njsh$ "
)

# Run test cases
for i in "${!test_cases[@]}"; do
    run_test "${test_cases[$i]}" "${expected_outputs[$i]}"
done