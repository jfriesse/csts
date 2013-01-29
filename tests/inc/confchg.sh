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
        run_app "$node" 'cpg-confchg' > "$test_var_dir/cpg-confchg-$node.log" &
        echo $! > "$test_var_dir/cpg-confchg-$node.pid"
    done

    confchg_used=true

    return 0
}

# confchg_checkview node expected_nodes
confchg_checkview() {
    local node="$1"
    local expected_nodes="$2"
    local no_retries=0

    while ! tail -1 "$test_var_dir/cpg-confchg-$node.log" | grep "^[0-9T]*:VIEW:$expected_nodes:" &>/dev/null && [ $no_retries -lt 40 ];do
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
        if [ -f "$test_var_dir/cpg-confchg-$node.pid" ];then
            kill -INT `cat "$test_var_dir/cpg-confchg-$node.pid"` &>/dev/null || true
            rm "$test_var_dir/cpg-confchg-$node.pid" || true
            cat "$test_var_dir/cpg-confchg-$node.log" || true
        fi
    done

    return 0
}
