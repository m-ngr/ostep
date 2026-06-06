#!/bin/bash

# Exit if any command fails
set -e

# Base directory
INPUTS_DIR="#inputs"

# Recreate an empty inputs directory structure
rm -rf "$INPUTS_DIR"
mkdir -p "$INPUTS_DIR"/set1 "$INPUTS_DIR"/set2 "$INPUTS_DIR"/set3 "$INPUTS_DIR"/set4

echo "=== Generating Test Sets ==="

# Dictionary of words to randomly pull from
WORDS=("apple" "banana" "cherry" "date" "elderberry" "fig" "grape" "honeydew" "kiwi" "lemon" "mango" "orange" "papaya" "quince" "raspberry" "strawberry" "tangerine" "ugli" "vanilla" "watermelon")

# Function to generate a file with random words
# Usage: generate_file <path> <num_lines> <words_per_line>
generate_file() {
    local filepath=$1
    local lines=$2
    local wpl=$3
    
    # Empty out or create file
    > "$filepath"
    
    for ((i=0; i<lines; i++)); do
        local line=""
        for ((j=0; j<wpl; j++)); do
            # Pick a random word from our array
            local rand_idx=$(( RANDOM % ${#WORDS[@]} ))
            line+="${WORDS[$rand_idx]} "
        done
        echo "$line" >> "$filepath"
    done
}

# --- Set 1: Small text, few files (< 4 files, ~5 lines each) ---
echo "Generating Set 1..."
for i in {1..3}; do
    generate_file "$INPUTS_DIR/set1/small_few_$i.txt" 5 4
done

# --- Set 2: Small text, many files (> 5 files, ~5 lines each) ---
echo "Generating Set 2..."
for i in {1..8}; do
    generate_file "$INPUTS_DIR/set2/small_many_$i.txt" 5 4
done

# --- Set 3: Large text, few files (< 4 files, ~2000 lines each) ---
echo "Generating Set 3..."
for i in {1..3}; do
    generate_file "$INPUTS_DIR/set3/large_few_$i.txt" 2000 10
done

# --- Set 4: Large text, many files (> 5 files, ~1000 lines each) ---
echo "Generating Set 4..."
for i in {1..8}; do
    generate_file "$INPUTS_DIR/set4/large_many_$i.txt" 1000 10
done

echo "=== Input Generation Complete! ==="