#!/bin/bash

PROGRAM=./codexion

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
NC="\033[0m"

run_test()
{
    name="$1"
    shift

    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$name${NC}"
    echo -e "${BLUE}Command: $PROGRAM $*${NC}"
    echo -e "${BLUE}========================================${NC}"

    output=$(mktemp)

    $PROGRAM "$@" > "$output"

    if grep -q "burned out" "$output"; then
        echo -e "${RED}[FAIL] Burnout detected${NC}"
    else
        echo -e "${GREEN}[PASS] No burnout${NC}"
    fi

    if grep -q "has taken a dongle" "$output"; then
        echo -e "${GREEN}[PASS] Dongle activity detected${NC}"
    else
        echo -e "${RED}[FAIL] No dongle activity${NC}"
    fi

    if grep -q "is compiling" "$output"; then
        echo -e "${GREEN}[PASS] Compile phase detected${NC}"
    else
        echo -e "${RED}[FAIL] No compile phase${NC}"
    fi

    if grep -q "is debugging" "$output"; then
        echo -e "${GREEN}[PASS] Debug phase detected${NC}"
    else
        echo -e "${RED}[FAIL] No debug phase${NC}"
    fi

    if grep -q "is refactoring" "$output"; then
        echo -e "${GREEN}[PASS] Refactor phase detected${NC}"
    else
        echo -e "${RED}[FAIL] No refactor phase${NC}"
    fi

    echo
    echo "----- Last 20 lines -----"
    tail -20 "$output"

    rm -f "$output"
}

echo -e "${YELLOW}========== FIFO : Cooldown 400 ==========${NC}"
run_test "FIFO 400" \
5 3000 200 200 200 10 400 fifo

echo -e "${YELLOW}========== FIFO : Cooldown 800 ==========${NC}"
run_test "FIFO 800" \
5 3000 200 200 200 10 800 fifo

echo -e "${YELLOW}========== EDF : Cooldown 400 ==========${NC}"
run_test "EDF 400" \
5 3000 200 200 200 10 400 edf

echo -e "${YELLOW}========== EDF : Cooldown 800 ==========${NC}"
run_test "EDF 800" \
5 3000 200 200 200 10 800 edf

echo
echo -e "${BLUE}========== Stress Test (20 runs each) ==========${NC}"

stress()
{
    title="$1"
    shift

    echo
    echo -e "${YELLOW}$title${NC}"

    for ((i=1;i<=20;i++))
    do
        if $PROGRAM "$@" | grep -q "burned out"; then
            echo -e "${RED}Run $i : FAIL${NC}"
            return
        fi

        printf "."
    done

    echo
    echo -e "${GREEN}20/20 successful${NC}"
}

stress "FIFO cooldown 400" \
5 3000 200 200 200 10 400 fifo

stress "FIFO cooldown 800" \
5 3000 200 200 200 10 800 fifo

stress "EDF cooldown 400" \
5 3000 200 200 200 10 400 edf

stress "EDF cooldown 800" \
5 3000 200 200 200 10 800 edf