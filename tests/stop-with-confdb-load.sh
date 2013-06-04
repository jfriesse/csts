#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that stopping corosync with confdb load works"

. inc/common.sh

compile_confdb_app "$nodes_ip" "confdb-track-and-change"

configure_corosync "$nodes_ip"

for ((i=0; i<5; i++));do
    start_corosync "$nodes_ip" "-p"

    run_app "$nodes_ip" 'confdb-track-and-change -u -n 1 -c 64' &
    pid=$!
    sleep 10

    stop_corosync "$nodes_ip"

    wait $pid
done

exit 0
