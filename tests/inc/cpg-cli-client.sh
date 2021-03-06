#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

cpg_cli_client_used=false

# cpg_cli_client_prepare nodes
cpg_cli_client_prepare() {
    local nodes="$1"
    local node

    for node in $nodes;do
        compile_app "$node" "cpg-cli-client" "-lcpg" || return $?
    done

    driver_compile_app "stdin-to-usock" || return $?
    driver_compile_app "usock-to-stdout" || return $?

    return 0
}

# cpg_cli_client_send_run node
cpg_cli_client_send_run() {
    local node="$1"

    driver_run_app "stdin-to-usock" "$test_var_dir/cpg-cli-client-$node.sock"
}

# cpg_cli_client_exit node
cpg_cli_client_exit() {
    local node="$1"

    echo -n "" | cpg_cli_client_send_run "$node"
}

# cpg_cli_client_cat_log node
cpg_cli_client_cat_log() {
    local node="$1"

    cat "$test_var_dir/cpg-cli-client-$node.log"
}

# cpg_cli_client_send_msg node msg
cpg_cli_client_send_msg() {
    local node="$1"
    local msg="$2"

    echo "sendstr $msg" | cpg_cli_client_send_run "$node"
}


# cpg_cli_client_wait_for_msg nodes msg
cpg_cli_client_wait_for_msg() {
    local nodes="$1"
    local msg="$2"
    local no_retries=0
    local node

    for node in $nodes;do
        while ! cpg_cli_client_cat_log "$node" | \
	  grep "^[0-9T]*:Arrived:([0-9a-z]* [0-9a-z]*):STR:[0-9]*:[0-9a-z]*:$msg\$" &>/dev/null && [ $no_retries -lt 40 ];do
            sleep 1
            no_retries=$(($no_retries + 1))
        done

        [ "$no_retries" -lt 40 ] && true || return 1
    done

    return 0
}

# cpg_cli_client_wait_for_start node
cpg_cli_client_wait_for_start() {
    local node="$1"
    local no_retries=0

    while ! cpg_cli_client_cat_log "$node" | grep "^[0-9T]*:ConfchgCallback:" &>/dev/null && [ $no_retries -lt 40 ];do
        sleep 1
        no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -lt 40 ] && return 0 || return 1
}

# cpg_cli_client_start nodes
cpg_cli_client_start() {
    local nodes="$1"
    local node

    cpg_cli_client_used=true

    for node in $nodes;do
        driver_run_app "usock-to-stdout" "$test_var_dir/cpg-cli-client-$node.sock" | \
            run_app "$node" "cpg-cli-client" > "$test_var_dir/cpg-cli-client-$node.log" &
        echo $! > "$test_var_dir/cpg-cli-client-$node.pid"
        cpg_cli_client_wait_for_start "$node" || return $?
    done

    return 0
}

# cpg_cli_client_stop
cpg_cli_client_stop() {
    local nodes="$1"
    local node

    for node in $nodes;do
        if [ -f "$test_var_dir/cpg-cli-client-$node.pid" ];then
            cpg_cli_client_exit "$node"
            pkill -P "`cat \"$test_var_dir/cpg-cli-client-$node.pid\"`" || true
            wait "`cat \"$test_var_dir/cpg-cli-client-$node.pid\"`" || true
            rm "$test_var_dir/cpg-cli-client-$node.pid" || true
        fi
    done

    return 0
}

# cpg_cli_client_last_confchg node
cpg_cli_client_last_confchg() {
    local node="$1"

    cpg_cli_client_cat_log "$node" | tac | grep "^[0-9T]*:ConfchgCallback:" | head -1
}

# cpg_cli_client_wait_for_last_confchg_no_members nodes members
cpg_cli_client_wait_for_last_confchg_no_members() {
    local nodes="$1"
    local members="$2"
    local no_retries=0
    local node

    for node in $nodes;do
        while ! cpg_cli_client_last_confchg "$node" | \
	  grep "^[0-9T]*:ConfchgCallback:[0-9a-zA-Z]*:[0-9]*,[0-9]*,$members:" &>/dev/null && [ $no_retries -lt 40 ];do
            sleep 1
            no_retries=$(($no_retries + 1))
        done

        [ "$no_retries" -lt 40 ] && true || return 1
    done

    return 0
}

# cpg_cli_client_listmem node
cpg_cli_client_listmem() {
    local node="$1"
    local usernum=$RANDOM
    local no_retries=0

    echo "listmem $usernum" | cpg_cli_client_send_run "$node"

    while ! cpg_cli_client_cat_log "$node" | tac | grep "^[0-9T]*:ListMem:$usernum:" &>/dev/null \
        && [ $no_retries -lt 40 ];do
            sleep 1
            no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -lt 40 ] && true || return 1

    cpg_cli_client_cat_log "$node" | tac | grep "^[0-9T]*:ListMem:$usernum:" | head -1 | \
	sed 's/^[0-9T]*:ListMem:'$usernum'://'

    return 0
}

# cpg_cli_client_wait_for_last_confchg_in_sync master_node other_nodes
cpg_cli_client_wait_for_last_confchg_in_sync() {
    local master_node="$1"
    local other_nodes="$2"

    local no_retries=0
    local node
    local membership

    membership=`cpg_cli_client_listmem "$master_node"`

    for node in $other_nodes;do
	while [ "`cpg_cli_client_listmem $node`" != "$membership" ] && [ $no_retries -lt 40 ];do
            sleep 1
            no_retries=$(($no_retries + 1))
        done

        [ "$no_retries" -lt 40 ] && true || return 1
    done

    return 0
}
