#!/bin/bash

# Default to C if no version is specified, or grab the first argument
VERSION=${1:-"c"}

# Shift the arguments so that "$*" or "$@" down below only contains 
# the flags meant for the tester script (e.g., -v, SpecificTestNum)
if [[ "$VERSION" == "go" || "$VERSION" == "c" ]]; then
    shift
else
    # If the first argument wasn't 'go' or 'c', default to 'c' and don't shift
    VERSION="c"
fi

echo "Building and testing the ${VERSION^^} version..."

if [ "$VERSION" = "go" ]; then
    # Compile the Go version from the cmd directory
    cd ./go
    go build -o ../pzip
    cd ..
else
    # Compile the C files from the src directory
    gcc src/main.c src/mmap_helper.c src/work_queue.c src/processor.c -o pzip -lpthread -O3
fi

# Double check that the compilation actually succeeded and generated the binary
if ! [[ -x pzip ]]; then
    echo "pzip executable does not exist or compilation failed"
    exit 1
fi

# Run your automated grading/tester script with any remaining arguments
../tester/run-tests.sh "$@"