#!/bin/bash

PROGRAM="./codexion"
LOG_DIR="test_logs"

mkdir -p "$LOG_DIR"

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

pass(){ echo -e "${GREEN}[PASS]${NC} $1"; }
fail(){ echo -e "${RED}[FAIL]${NC} $1"; }
info(){ echo -e "${BLUE}[INFO]${NC} $1"; }

############################################

check_log()
{
    local log="$1"

    ##################################
    # compile must have two dongles
    ##################################

    awk '

    {
        id=$2

        if($3=="has")
            taken[id]++

        else if($3=="is" && $4=="compiling")
        {
            if(taken[id]!=2)
                bad=1

            taken[id]=0
        }
    }

    END{
        exit bad
    }

    ' "$log"

    if [ $? -eq 0 ]; then
        pass "Two-dongle rule"
    else
        fail "Compile without exactly two dongles"
    fi

    ##################################
    # burnout last line
    ##################################

    if grep -q "burned out" "$log"
    then

        burn=$(grep -n "burned out" "$log" | cut -d: -f1)

        total=$(wc -l < "$log")

        if [ "$burn" -eq "$total" ]
        then
            pass "Burnout is last line"
        else
            fail "Output after burnout"
        fi
    fi

    ##################################
    # merged lines
    ##################################

    if grep -Eq "[0-9]+ .* [0-9]+ .*has taken" "$log"
    then
        fail "Merged log lines"
    else
        pass "Serialized output"
    fi
}

############################################

run_case()
{
    local name="$1"
    local args="$2"
    local expected="$3"
    local repeat="$4"

    echo
    echo "======================================"
    echo "$name"
    echo "$PROGRAM $args"
    echo "======================================"

    ok=1

    for ((i=1;i<=repeat;i++))
    do
        log="$LOG_DIR/${name}_$i.log"

        timeout 30s $PROGRAM $args > "$log"

        if [ $? -eq 124 ]
        then
            fail "Timeout (run $i)"
            ok=0
            continue
        fi

        if [ "$expected" = burnout ]
        then
            if ! grep -q "burned out" "$log"
            then
                fail "Expected burnout (run $i)"
                ok=0
            fi
        else
            if grep -q "burned out" "$log"
            then
                fail "Unexpected burnout (run $i)"
                ok=0
            fi
        fi

        check_log "$log"

    done

    [ $ok -eq 1 ] && pass "$name"
}

#########################################################
echo
echo "========== EASY =========="

run_case \
single \
"1 800 200 200 200 10 0 fifo" \
burnout \
5

run_case \
fifo_easy \
"5 2000 200 200 200 10 0 fifo" \
success \
20

run_case \
edf_easy \
"5 2000 200 200 200 7 0 edf" \
success \
20

#########################################################

echo
echo "========== LESS EASY =========="

run_case \
burnout_edge \
"5 500 200 200 200 10 0 fifo" \
burnout \
5

#########################################################

echo
echo "========== MEDIUM =========="

run_case \
cooldown_fifo \
"5 3000 200 200 200 10 400 fifo" \
success \
20

run_case \
fifo_contention \
"5 3000 200 200 200 10 800 fifo" \
success \
20

run_case \
edf_contention \
"5 3000 200 200 200 10 800 edf" \
success \
20

#########################################################

echo
echo "========== LARGE =========="

run_case \
large \
"50 5000 100 100 100 10 200 fifo" \
success \
10

#########################################################

echo
echo "========== VALGRIND =========="

valgrind \
--leak-check=full \
--show-leak-kinds=all \
--errors-for-leak-kinds=all \
$PROGRAM \
5 2000 200 200 200 10 0 fifo \
> "$LOG_DIR/valgrind.log" 2>&1

grep -q "ERROR SUMMARY: 0 errors" "$LOG_DIR/valgrind.log" \
&& pass "Valgrind" \
|| fail "Valgrind"

#########################################################

echo
echo "========== HELGRIND =========="

valgrind \
--tool=helgrind \
$PROGRAM \
5 2000 200 200 200 10 0 fifo \
> "$LOG_DIR/helgrind.log" 2>&1

grep -q "ERROR SUMMARY: 0 errors" "$LOG_DIR/helgrind.log" \
&& pass "Helgrind" \
|| fail "Helgrind"

echo
info "Logs stored in $LOG_DIR"