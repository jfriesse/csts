#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that start/stop in parallel cycle works"
test_required_nodes=3
test_max_nodes=-1
test_max_runtime=600

. inc/common.sh

pids=""

for node in $nodes_ip;do
    (configure_corosync "$node"
     for ((i=0; i<100; i++));do
         start_corosync "$node"
         sleep 0.0$(($RANDOM % 10))
         stop_corosync "$node"
     done) &
    pids="$! $pids"
done

strict_wait $pids

exit 0
