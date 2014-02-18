#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that cpg properly detect left pid after pause"
test_required_nodes=3
test_max_nodes=3
test_max_runtime=600

. inc/common.sh
. inc/cpg-cli-client.sh

cpg_cli_client_prepare "$nodes_ip"

permuted_nodes=`permute_word_order "$nodes_ip" | tr ' \n' ': '`

for tc in $permuted_nodes;do
    nodes_array=( `echo $tc | tr ':' ' '` )

    for node in $nodes_ip;do
        configure_corosync "$node"
        start_corosync "$node"
    done
    cpg_cli_client_start "$nodes_ip"

    pause_corosync "${nodes_array[0]}"
    cpg_cli_client_wait_for_last_confchg_no_members "${nodes_array[1]} ${nodes_array[2]}" 2

    cpg_cli_client_stop "${nodes_array[1]}"
    cpg_cli_client_wait_for_last_confchg_no_members "${nodes_array[2]}" 1

    cpg_cli_client_start "${nodes_array[1]}"
    cpg_cli_client_wait_for_last_confchg_no_members "${nodes_array[2]}" 2

    unpause_corosync "${nodes_array[0]}"
    cpg_cli_client_wait_for_last_confchg_no_members "${nodes_array[1]} ${nodes_array[2]}" 3
    sleep 2
    cpg_cli_client_wait_for_last_confchg_no_members "${nodes_array[0]}" 3

    cpg_cli_client_stop "$nodes_ip"
    for node in $nodes_ip;do
        stop_corosync "$node"
    done
done

exit 0
