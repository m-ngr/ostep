#!/bin/bash

# Define terminal color codes
RED="\033[0;31m"
GREEN="\033[0;32m"
NC="\033[0m" # No Color

echo "=== Step 1: Compiling Code ==="
make clean
make

if [ ! -d "inputs/set1" ]; then
    echo "Inputs folder missing. Generating..."
    ./generate_inputs.sh
fi

# Summary tracking variables
SET1_CORR="?" ; SET1_MEM="?"
SET2_CORR="?" ; SET2_MEM="?"
SET3_CORR="?" ; SET3_MEM="?"
SET4_CORR="?" ; SET4_MEM="?"

# Core processing function
run_matrix() {
    local files_path=$1
    local log_prefix=$2
    
    # 1. Correctness Test
    cat $files_path | tr -s ' \t\r' '\n' | grep -v '^$' | sort | uniq -c | awk '{print $2 " " $1}' | sort > expected.txt
    ./wordcount $files_path > raw_output.txt 2>/dev/null
    sort raw_output.txt > output.txt 2>/dev/null
    rm -f raw_output.txt

    if [ -s output.txt ] && diff -q output.txt expected.txt > /dev/null; then
        corr_res="PASS"
        corr_log="${GREEN}PASS${NC}"
    else
        corr_res="FAIL"
        corr_log="${RED}FAIL${NC}"
    fi
    rm -f expected.txt output.txt

    # 2. Memory Test
    valgrind --leak-check=full ./wordcount $files_path > /dev/null 2> valgrind_log.txt
    
    if grep -q "ERROR SUMMARY: 0 errors" valgrind_log.txt && grep -q "definitely lost: 0 bytes" valgrind_log.txt; then
        mem_res="PASS"
        mem_log="${GREEN}PASS${NC}"
        rm -f valgrind_log.txt
    else
        mem_res="FAIL"
        mem_log="${RED}FAIL${NC}"
        mv valgrind_log.txt "${log_prefix}_valgrind_failed.txt"
    fi
}

# --- Run & Log on 1 Clean Line Per Set ---
run_matrix "inputs/set1/*.txt" "set1"
SET1_CORR=$corr_res; SET1_MEM=$mem_res
echo -e "SET 1 (Small text, few files): Correctness: $corr_log, Memory: $mem_log"

run_matrix "inputs/set2/*.txt" "set2"
SET2_CORR=$corr_res; SET2_MEM=$mem_res
echo -e "SET 2 (Small text, many files): Correctness: $corr_log, Memory: $mem_log"

run_matrix "inputs/set3/*.txt" "set3"
SET3_CORR=$corr_res; SET3_MEM=$mem_res
echo -e "SET 3 (Large text, few files): Correctness: $corr_log, Memory: $mem_log"

run_matrix "inputs/set4/*.txt" "set4"
SET4_CORR=$corr_res; SET4_MEM=$mem_res
echo -e "SET 4 (Large text, many files): Correctness: $corr_log, Memory: $mem_log"

# --- Helper function for Table Color Injection ---
table_format() {
    if [ "$1" == "PASS" ]; then echo -e "${GREEN}PASS${NC}"; else echo -e "${RED}FAIL${NC}"; fi
}

S1_C=$(table_format $SET1_CORR) ; S1_M=$(table_format $SET1_MEM)
S2_C=$(table_format $SET2_CORR) ; S2_M=$(table_format $SET2_MEM)
S3_C=$(table_format $SET3_CORR) ; S3_M=$(table_format $SET3_MEM)
S4_C=$(table_format $SET4_CORR) ; S4_M=$(table_format $SET4_MEM)

# --- Final Table Summary ---
echo -e "\n======================================================="
echo -e "                 MAPREDUCE TEST SUMMARY                "
echo -e "======================================================="
printf "| %-30s | %-11s | %-11s |\n" "TEST MATRIX SUITE" "CORRECTNESS" "MEMORY LEAK"
echo -e "-------------------------------------------------------"
printf "| %-30s | %-20s | %-20s |\n" "SET 1: Small Text, Few Files" "$S1_C" "$S1_M"
printf "| %-30s | %-20s | %-20s |\n" "SET 2: Small Text, Many Files" "$S2_C" "$S2_M"
printf "| %-30s | %-20s | %-20s |\n" "SET 3: Large Text, Few Files" "$S3_C" "$S3_M"
printf "| %-30s | %-20s | %-20s |\n" "SET 4: Large Text, Many Files" "$S4_C" "$S4_M"
echo -e "======================================================="

if [ "$SET1_MEM" == "FAIL" ] || [ "$SET2_MEM" == "FAIL" ] || [ "$SET3_MEM" == "FAIL" ] || [ "$SET4_MEM" == "FAIL" ]; then
    echo -e "\nNote: Failed traces saved to '*_valgrind_failed.txt'"
fi