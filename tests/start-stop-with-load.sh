#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that start/stop in parallel cycle works with cpgload executed"
test_required_nodes=3
test_max_nodes=-1
test_max_runtime=600

. inc/common.sh
. inc/cpg-load.sh

pids=""

for node in $nodes_ip;do
    cpg_load_prepare "$node"
    (configure_corosync "$node"
     for ((i=0; i<50; i++));do
         start_corosync "$node"

         cpg_load_start "$node" "10"

         sleep 0.$(($RANDOM % 10))

         stop_corosync "$node"

         cpg_load_verify "$node"
         cpg_load_stop "$node"
     done) &
    pids="$! $pids"
done

strict_wait $pids

exit 0
