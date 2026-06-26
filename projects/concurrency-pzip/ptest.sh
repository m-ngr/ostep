#!/usr/bin/env bash

###############################################################################
# Parallel PZip Test Framework
#
# Commands:
#
#   ./ptest.sh gen
#   ./ptest.sh c
#   ./ptest.sh go
#   ./ptest.sh diff
#   ./ptest.sh perf
#   ./ptest.sh clean
#   ./ptest.sh all
#
###############################################################################

set -euo pipefail

################################################################################
# Configuration
################################################################################

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GO_DIR="$ROOT/go"
C_DIR="$ROOT/src"

GO_BIN="$ROOT/pzip-go"
C_BIN="$ROOT/pzip-c"

DATA_ROOT="$ROOT"

SUITE_DIR="$DATA_ROOT/test-suits"
RESULT_DIR="$SUITE_DIR/results"

LARGE_SIZE=1221225472

################################################################################
# Colors
################################################################################

RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
CYAN="\033[36m"
RESET="\033[0m"

################################################################################
# Helpers
################################################################################

info() {
    echo -e "${CYAN}$*${RESET}"
}

warn() {
    echo -e "${YELLOW}$*${RESET}"
}

success() {
    echo -e "${GREEN}$*${RESET}"
}

error() {
    echo -e "${RED}$*${RESET}"
    exit 1
}

################################################################################
# Usage
################################################################################

usage() {

cat <<EOF

Usage:

    ./ptest.sh gen
    ./ptest.sh c
    ./ptest.sh go
    ./ptest.sh diff
    ./ptest.sh perf
    ./ptest.sh clean
    ./ptest.sh all

Commands

gen      Generate stress test suite.
c        Build C implementation and run all tests.
go       Build Go implementation and run all tests.
diff     Compare C and Go outputs.
perf     Compare execution times.
clean    Remove generated files.
all      Execute everything.

EOF

}

################################################################################
# Test Data Helpers
################################################################################

ensure_storage() {

    mkdir -p "$DATA_ROOT"

    if [[ ! -d "$DATA_ROOT" ]]; then
        error "Storage path does not exist: $DATA_ROOT"
    fi

    if [[ ! -w "$DATA_ROOT" ]]; then
        error "Storage path is not writable: $DATA_ROOT"
    fi

}

create_random_file() {

    local file="$1"
    local target_size="$2"

    info "Generating $(basename "$file")"

    mkdir -p "$(dirname "$file")"

    python3 - "$file" "$target_size" <<'PY'
import os
import random
import string
import sys

outfile = sys.argv[1]
target = int(sys.argv[2])

remaining = target

with open(outfile, "wb") as f:

    while remaining > 0:

        # Random character A-Z
        ch = random.choice(string.ascii_uppercase).encode()

        # Random run length (1 .. 1 MiB)
        run = random.randint(1, 1024 * 1024)

        if run > remaining:
            run = remaining

        f.write(ch * run)

        remaining -= run

PY

}

create_zero_file() {

    local file="$1"
    local size="$2"

    info "Generating $(basename "$file")"

    mkdir -p "$(dirname "$file")"

    truncate -s "$size" "$file"

}

################################################################################
# Cleanup
################################################################################

clean() {

    warn "Cleaning generated artifacts..."

    rm -rf "$SUITE_DIR"

    rm -f "$GO_BIN"
    rm -f "$C_BIN"

    success "Done."

}

################################################################################
# Generate Test Suite
################################################################################

generate_tests() {
    
    ensure_storage

    warn "Removing old suite..."

    rm -rf "$SUITE_DIR"

    mkdir -p "$RESULT_DIR"

    #
    # Test directories
    #

    for i in {1..6}; do

        mkdir -p "$SUITE_DIR/test$i"
        mkdir -p "$RESULT_DIR/test$i"

    done

    ############################################################################
    # Test 1
    ############################################################################

    printf "AAAAABBBCC" \
        > "$SUITE_DIR/test1/f1.txt"

    ############################################################################
    # Test 2
    ############################################################################

    printf "AAAA" \
        > "$SUITE_DIR/test2/f1.txt"

    printf "BBBB" \
        > "$SUITE_DIR/test2/f2.txt"

    printf "CCCC" \
        > "$SUITE_DIR/test2/f3.txt"

    ############################################################################
    # Test 3
    ############################################################################

    printf "AAAAABBB" \
        > "$SUITE_DIR/test3/f1.txt"

    printf "BBCCCCDD" \
        > "$SUITE_DIR/test3/f2.txt"

    printf "DDEEEEE" \
        > "$SUITE_DIR/test3/f3.txt"

    ############################################################################
    # Test 4
    ############################################################################

    for i in $(seq 1 100)
    do
        c=$((i % 3))

        case $c in
            0) ch="A" ;;
            1) ch="B" ;;
            2) ch="C" ;;
        esac

        printf "%s%s%s" "$ch" "$ch" "$ch" \
            > "$SUITE_DIR/test4/f$i.txt"

    done

    ############################################################################
    # Test 5
    ############################################################################

    info "Generating Test 5 (4 × 3GB random)"

    for i in {1..4}
    do
        create_random_file \
            "$SUITE_DIR/test5/huge$i.dat" \
            "$LARGE_SIZE"
    done

    ############################################################################
    # Test 6
    ############################################################################

    info "Generating Test 6 (5 × zero + 15 × random)"

    for i in $(seq 1 10)
    do

        if (( i <= 5 ))
        then

            create_zero_file \
                "$SUITE_DIR/test6/large$i.dat" \
                "$LARGE_SIZE"

        else

            create_random_file \
                "$SUITE_DIR/test6/large$i.dat" \
                "$LARGE_SIZE"

        fi

    done

    success "Test suite generated successfully."

}

################################################################################
# Build
################################################################################

build_c() {

    info "Building C implementation..."

    gcc \
        "$C_DIR/main.c" \
        "$C_DIR/mmap_helper.c" \
        "$C_DIR/work_queue.c" \
        "$C_DIR/processor.c" \
        -o "$C_BIN" \
        -lpthread \
        -O3 \
        -mcmodel=large

    success "Built $C_BIN"

}

build_go() {

    info "Building Go implementation..."

    (
        cd "$GO_DIR"
        go build -o "$GO_BIN"
    )

    success "Built $GO_BIN"

}

################################################################################
# Time helper
################################################################################

now_ns() {
    date +%s%N
}

################################################################################
# Run Tests
################################################################################

run_tests() {

    local bin="$1"
    local prefix="$2"

    [[ -x "$bin" ]] || error "Executable not found: $bin"

    for i in {1..6}
    do

        info "Running Test $i..."

        local test_dir="$SUITE_DIR/test$i"
        local result_dir="$RESULT_DIR/test$i"

        local output_file="$result_dir/${prefix}-result"
        local duration_file="$result_dir/${prefix}-duration"

        rm -f "$output_file"

        #
        # Build input file list in sorted order.
        #

        mapfile -t files < <(
            find "$test_dir" \
                -maxdepth 1 \
                -type f \
                | sort
        )

        if [[ ${#files[@]} -eq 0 ]]
        then
            error "No files found in $test_dir"
        fi

        start=$(now_ns)

        "$bin" "${files[@]}" > "$output_file"

        end=$(now_ns)

        echo $((end - start)) > "$duration_file"

        success "Finished Test $i"

    done

}

################################################################################
# Wrappers
################################################################################

run_c() {

    build_c

    run_tests \
        "$C_BIN" \
        "c"

}

run_go() {

    build_go

    run_tests \
        "$GO_BIN" \
        "go"

}

################################################################################
# Diff
################################################################################

show_diff() {

    printf "\n"

    printf "%-8s %-10s %-40s %-40s\n" \
        "Test" \
        "Status" \
        "C Result" \
        "Go Result"

    printf "%-8s %-10s %-40s %-40s\n" \
        "--------" \
        "----------" \
        "----------------------------------------" \
        "----------------------------------------"

    for i in {1..6}
    do

        cfile="$RESULT_DIR/test$i/c-result"
        gofile="$RESULT_DIR/test$i/go-result"

        if [[ ! -f "$cfile" || ! -f "$gofile" ]]
        then
            printf "%-8s ${RED}%-10s${RESET}\n" \
                "Test $i" \
                "MISSING"
            continue
        fi

        if cmp -s "$cfile" "$gofile"
        then
            status="${GREEN}SAME${RESET}"
        else
            status="${RED}DIFF${RESET}"
        fi

        printf "%-8s %-19b %-40s %-40s\n" \
            "Test $i" \
            "$status" \
            "$cfile" \
            "$gofile"

    done

}

################################################################################
# Performance
################################################################################

show_perf() {

    printf "\n"

    printf "%-8s %-10s %-12s %-12s %-12s %-28s\n" \
        "Test" \
        "Status" \
        "C(ms)" \
        "Go(ms)" \
        "Diff(ms)" \
        "Performance"

    printf "%-8s %-10s %-12s %-12s %-12s %-28s\n" \
        "--------" \
        "----------" \
        "------------" \
        "------------" \
        "------------" \
        "----------------------------"

    for i in {1..6}
    do

        cfile="$RESULT_DIR/test$i/c-result"
        gofile="$RESULT_DIR/test$i/go-result"

        cdur=$(cat "$RESULT_DIR/test$i/c-duration")
        godur=$(cat "$RESULT_DIR/test$i/go-duration")

        c_ms=$(awk "BEGIN { printf \"%.3f\", $cdur/1000000 }")
        go_ms=$(awk "BEGIN { printf \"%.3f\", $godur/1000000 }")

        diff_ms=$(awk "BEGIN {
            d=($cdur-$godur);
            if(d<0)d=-d;
            printf \"%.3f\", d/1000000
        }")

        if cmp -s "$cfile" "$gofile"
        then
            status="${GREEN}SAME${RESET}"
        else
            status="${RED}DIFF${RESET}"
        fi

        if (( godur < cdur ))
        then

            percent=$(awk "BEGIN {
                printf \"%.2f\", (($cdur-$godur)/$cdur)*100
            }")

            perf="Go faster by ${percent}%"

        elif (( cdur < godur ))
        then

            percent=$(awk "BEGIN {
                printf \"%.2f\", (($godur-$cdur)/$godur)*100
            }")

            perf="C faster by ${percent}%"

        else

            perf="Equal"

        fi

        printf "%-8s %-19b %-12s %-12s %-12s %-28s\n" \
            "Test $i" \
            "$status" \
            "$c_ms" \
            "$go_ms" \
            "$diff_ms" \
            "$perf"

    done

}

################################################################################
# All
################################################################################

run_all() {

    generate_tests

    run_c

    run_go

    show_diff

    show_perf

}

################################################################################
# Main
################################################################################

case "${1:-}" in

    gen)
        generate_tests
        ;;

    c)
        run_c
        ;;

    go)
        run_go
        ;;

    diff)
        show_diff
        ;;

    perf)
        show_perf
        ;;

    clean)
        clean
        ;;

    all)
        run_all
        ;;

    *)
        usage
        exit 1
        ;;

esac