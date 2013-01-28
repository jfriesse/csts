#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test cpg - change of configuration"
test_required_nodes=3
test_max_nodes=-1
test_max_runtime=600

. inc/common.sh
. inc/fw.sh
. inc/confchg.sh

declare -A node_status

active_nodes=0

exit_trap_end_cb() {
    fw_unblock_udp "$nodes_ip" true
}

check_views() {
    local no_nodes
    local no_retries=0

    for n in $nodes_ip;do
        [ ${node_status["$n"]} == "up" ] && no_nodes=$active_nodes || no_nodes=1

        confchg_checkview "$n" "$no_nodes" || return $?
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
        fw_block_udp "$node" || return $?
        node_status["$node"]="down"
        active_nodes=$(($active_nodes - 1))
    else
        fw_unblock_udp "$node" || return $?
        node_status["$node"]="up"
        active_nodes=$(($active_nodes + 1))
    fi

    check_views || return $?

    return 0
}

for node in $nodes_ip;do
    configure_corosync "$node"
    start_corosync "$node"
    confchg_start "$node"
    node_status["$node"]="up"
    active_nodes=$(($active_nodes + 1))
done

check_views || exit $?

for ((i=0; i<100; i++));do
    node_num=$(($RANDOM % $no_nodes + 1))
    node=`echo $nodes_ip | cut -d ' ' -f $node_num`
    change_node_state "$node"
done

exit 0
