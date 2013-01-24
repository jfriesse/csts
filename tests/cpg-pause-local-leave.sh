#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test cpg - pause and unpause of one node doesn't result leave of any other nodes"
test_required_nodes=2
test_max_nodes=2

. inc/common.sh
. inc/confchg.sh

for node in $nodes_ip;do
    configure_corosync "$node"
    start_corosync "$node"
    confchg_start "$node"
done

# Wait for two nodes to see each other
for ((i=0; i<$no_nodes; i++));do
    confchg_checkview "${nodes_ip_array[$i]}" "$no_nodes"
done

for ((i=0; i<$no_nodes; i++));do
    pause_corosync "${nodes_ip_array[$i]}"

    for ((j=0; j<$no_nodes; j++));do
        if [ "$j" != "$i" ];then
            confchg_checkview "${nodes_ip_array[$j]}" $(($no_nodes - 1))
        fi
    done

    unpause_corosync "${nodes_ip_array[$i]}"

    for ((j=0; j<$no_nodes; j++));do
        confchg_checkview "${nodes_ip_array[$j]}" "$no_nodes"
    done
done

exit 0
