#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that restart of nodes one by one works"
test_required_nodes=3
test_max_nodes=-1

. common.sh

no_cycles=25

for node in $nodes_ip;do
    configure_corosync "$node"
    start_corosync "$node"
done

for ((i=0; i<$no_cycles; i++));do
    onodes_ip=`randomize_word_order $nodes_ip`

    for node in $onodes_ip;do
        stop_corosync "$node"
        start_corosync "$node"
    done
done

exit 0
