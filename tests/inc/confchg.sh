#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

# confchg_start nodes
confchg_start() {
    local nodes="$1"
    local node

    for node in $nodes;do
        compile_app "$node" "cpg-confchg" "-lcpg" || return $?
        run_app "$node" "cpg-confchg > /tmp/cpg-confchg.log &" || return $?
    done

    return 0
}

# confchg_checkview node expected_nodes
confchg_checkview() {
    local node="$1"
    local expected_nodes="$2"
    local no_retries=0

    repeats=0
    while ! run "$node" "tail -1 /tmp/cpg-confchg.log" | grep "^[0-9T]*:VIEW:$expected_nodes:" &>/dev/null && [ $no_retries -lt 40 ];do
        sleep 0.5
        no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -ge 40 ] && return 1 || return 0
}
