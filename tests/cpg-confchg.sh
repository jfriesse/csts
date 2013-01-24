#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test cpg - change of configuration"
test_required_nodes=3
test_max_nodes=-1
test_max_runtime=600

. inc/common.sh

declare -A node_status
declare -A node_line

active_nodes=0

exit_trap_end_cb() {
    for node in $nodes_ip;do
        run "$node" "iptables -D INPUT ! -i lo -p udp -j DROP || true ; iptables -D OUTPUT ! -o lo -p udp -j DROP || true"
    done
}

check_views() {
    local no_nodes
    local no_retries=0

    for n in $nodes_ip;do
        [ ${node_status["$n"]} == "up" ] && no_nodes=$active_nodes || no_nodes=1

        repeats=0
        while ! run "$n" "tail -1 /tmp/cpg-confchg.log" | grep "^VIEW:$no_nodes:" &>/dev/null && [ $no_retries -lt 40 ];do
            sleep 0.5
            no_retries=$(($no_retries + 1))
        done

	[ "$no_retries" -ge 40 ] && return 1 || true
    done

    return 0
}

change_node_state() {
    local node="$1"
    local totem_change
    local totem_confchg_nodes
    local cpg_confchg_nodes
    local no_cbs

    if [ ${node_status["$node"]} == "up" ];then
        run "$node" "iptables -A INPUT ! -i lo -p udp -j DROP && iptables -A OUTPUT ! -o lo -p udp -j DROP"
        node_status["$node"]="down"
        active_nodes=$(($active_nodes - 1))
    else
        run "$node" "iptables -D INPUT ! -i lo -p udp -j DROP && iptables -D OUTPUT ! -o lo -p udp -j DROP"
        node_status["$node"]="up"
        active_nodes=$(($active_nodes + 1))
    fi

    check_views || return $?

    return 0
}

for node in $nodes_ip;do
    compile_app "$node" "cpg-confchg" "-lcpg"
    configure_corosync "$node"
    start_corosync "$node"
    run_app "$node" "cpg-confchg > /tmp/cpg-confchg.log &"
    node_status["$node"]="up"
    active_nodes=$(($active_nodes + 1))
done

check_views || return $?

for ((i=0; i<100; i++));do
    node_num=$(($RANDOM % $no_nodes + 1))
    node=`echo $nodes_ip | cut -d ' ' -f $node_num`
    change_node_state "$node"
done

exit 0
