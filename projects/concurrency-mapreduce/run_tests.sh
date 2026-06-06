#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "=== Step 1: Compiling Code ==="
make clean
make

echo "=== Step 2: Creating Ground Truth (Expected Output) ==="
# Based on our 3 input files, the total word count must be:
cat << EOF > expected.txt
apple 3
banana 2
cherry 3
EOF

echo "=== Step 3: Running Wordcount Program ==="
# We pass the input files to our executable
./wordcount inputs/file1.txt inputs/file2.txt inputs/file3.txt > raw_output.txt

# Sort the output because different Reducer threads might finish printing at different times
sort raw_output.txt > output.txt
rm raw_output.txt

echo "=== Step 4: Validating Output Correctness ==="
if diff -q output.txt expected.txt > /dev/null; then
    echo -e "\033[0;32m[PASS]\033[0m Correctness test succeeded!"
else
    echo -e "\033[0;31m[FAIL]\033[0m Output does not match expected counts."
    echo "--- Expected ---"
    cat expected.txt
    echo "--- Got ---"
    cat output.txt
    exit 1
fi

echo "=== Step 5: Checking for Memory Leaks (Valgrind) ==="
valgrind --leak-check=full --errors-to-stderr=yes ./wordcount inputs/file1.txt inputs/file2.txt inputs/file3.txt > /dev/null 2> valgrind_log.txt

if grep -q "ERROR SUMMARY: 0 errors" valgrind_log.txt && grep -q "definitely lost: 0 bytes" valgrind_log.txt; then
    echo -e "\033[0;32m[PASS]\033[0m No memory leaks or errors found!"
    rm valgrind_log.txt
else
    echo -e "\033[0;31m[FAIL]\033[0m Valgrind issues detected. Check 'valgrind_log.txt' for details."
    exit 1
fi

# Clean up temporary test files on complete success
rm expected.txt output.txt
echo -e "\n\033[0;32m=== ALL TESTS PASSED SUCCESSFULLY! ===\033[0m"