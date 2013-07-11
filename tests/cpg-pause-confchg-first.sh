#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test cpg - pause node, send message, unpause results in message delivered to both nodes after confchg"
test_required_nodes=2
test_max_nodes=2

. inc/common.sh
. inc/cpg-cli-client.sh

cpg_cli_client_prepare "$nodes_ip"

for node in $nodes_ip;do
    configure_corosync "$node"
    start_corosync "$node"
done

cpg_cli_client_start "$nodes_ip"

for ((i=0; i<$no_nodes; i++));do
    pause_corosync "${nodes_ip_array[$i]}"

    # Wait for corosync to really pause
    sleep 5

    cpg_cli_client_send_msg "${nodes_ip_array[$i]}" "PAUSE PONG $i"
    unpause_corosync "${nodes_ip_array[$i]}"
    cpg_cli_client_wait_for_msg "$nodes_ip" "PAUSE PONG $i"
done

exit 0
