#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

# cpg_load_prepare nodes
cpg_load_prepare() {
    local nodes="$1"
    local node

    for node in $nodes;do
        compile_app "$node" "cpg-load" "-lcpg" || return $?
    done

    return 0
}

# cpg_load_start nodes [no_messages_in_burst]
cpg_load_start() {
    local nodes="$1"
    local msgs="$2"
    local node

    if [ "$msgs" == "" ];then
        msgs=5000
    fi

    for node in $nodes;do
        run_app "$node" "cpg-load -n $msgs" > "$test_var_dir/cpg-load-stdout-$node.log" 2>"$test_var_dir/cpg-load-err-$node.log" &
        echo $! > "$test_var_dir/cpg-load-$node.pid"
    done

    return 0
}

# cpg_load_verify nodes
cpg_load_verify() {
    local nodes="$1"
    local node
    local err=0

    for node in $nodes;do
        if grep "^[0-9T]*:" "$test_var_dir/cpg-load-err-$node.log" &>/dev/null;then
            return 1
        fi
    done

    return 0
}


# cpg_load_stop
cpg_load_stop() {
    local nodes="$1"
    local node

    for node in $nodes;do
        if [ -f "$test_var_dir/cpg-load-$node.pid" ];then
            kill -INT `cat "$test_var_dir/cpg-load-$node.pid"` &>/dev/null || true
            rm "$test_var_dir/cpg-load-$node.pid" || true
        fi
    done

    return 0
}
