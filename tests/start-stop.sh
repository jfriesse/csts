#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that start/stop in parallel cycle works"
test_required_nodes=3
test_max_nodes=-1
test_max_runtime=1200

. common.sh

pids=""

for node in $nodes_ip;do
    (compile_app "$node" "testcpg" "-lcpg"
     configure_corosync "$node"
     for ((i=0; i<200; i++));do
         start_corosync "$node"

         no_retries=0
         while ! (echo "TEST"; echo "EXIT") | run_app "$node" 'testcpg' && [ $no_retries -lt 20 ]; do
             sleep 0.5
             no_retries=$(($no_retries + 1))
         done

         sleep 0.0$(($RANDOM % 10))

         stop_corosync "$node"
     done) &
    pids="$! $pids"
done

strict_wait $pids

exit 0
