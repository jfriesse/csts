#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that start and stop of all nodes at once works"
test_required_nodes=3
test_max_nodes=-1

. common.sh

no_cycles=50

for node in $nodes_ip;do
    configure_corosync "$node"
done

for ((i=0; i<$no_cycles; i++));do
    onodes_ip=`randomize_word_order $nodes_ip`

    pids=""
    for node in $onodes_ip;do
        start_corosync "$node" &
        pids="$! $pids"
    done
    strict_wait $pids

    pids=""
    for node in $onodes_ip;do
        stop_corosync "$node" &
        pids="$! $pids"
    done
    strict_wait $pids
done

exit 0
