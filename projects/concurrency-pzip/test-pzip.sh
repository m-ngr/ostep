#!/bin/bash

# Compile files from the src directory and output the binary to the project root
gcc src/main.c src/mmap_helper.c src/work_queue.c src/processor.c -o pzip -lpthread -O3

# Double check that the compilation actually succeeded and generated the binary
if ! [[ -x pzip ]]; then
    echo "pzip executable does not exist or compilation failed"
    exit 1
fi

# Run your automated grading/tester script
../tester/run-tests.sh $*