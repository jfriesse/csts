#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

confchg_used=false

# confchg_start nodes
confchg_start() {
    local nodes="$1"
    local node

    for node in $nodes;do
        compile_app "$node" "cpg-confchg" "-lcpg" || return $?
        run_app "$node" 'cpg-confchg > /tmp/cpg-confchg.log & echo $! > '"$test_var_dir/cpg-confchg.pid" || return $?
    done

    confchg_used=true

    return 0
}

# confchg_checkview node expected_nodes
confchg_checkview() {
    local node="$1"
    local expected_nodes="$2"
    local no_retries=0

    while ! run "$node" "tail -1 /tmp/cpg-confchg.log" | grep "^[0-9T]*:VIEW:$expected_nodes:" &>/dev/null && [ $no_retries -lt 40 ];do
        sleep 0.5
        no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -ge 40 ] && return 1 || return 0
}

# confchg_stop nodes
confchg_stop() {
    local nodes="$1"
    local node

    for node in $nodes;do
        if run "$node" "[ -f $test_var_dir/cpg-confchg.pid ]";then
            run "$node" "kill -INT "'`cat '"$test_var_dir"'/cpg-confchg.pid`'" &>/dev/null; rm $test_var_dir/cpg-confchg.pid"
            run "$node" "cat /tmp/cpg-confchg.log"
        fi
    done

    return 0
}
