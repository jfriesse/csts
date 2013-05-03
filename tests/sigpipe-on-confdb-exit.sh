#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that closing confdb client in connecting state doesn't end with SIGPIPE in daemon"

. inc/common.sh

compile_confdb_app "$nodes_ip" "confdb-track-and-change"

configure_corosync "$nodes_ip"

for ((i=0; i<10; i++));do
    # Start (foreground) corosync in background
    start_corosync_insert_marker "$nodes_ip"
    run $nodes_ip 'corosync -f -p' &
    coropid=$!
    start_corosync_wait_for_start "$nodes_ip"

    # Exec app
    run_app "$nodes_ip" 'confdb-track-and-change -u -n 1 -c 64' &
    pid=$!
    sleep 5

    # Kill app
    run "$nodes_ip" "killall -INT confdb-track-and-change"
    wait $pid

    stop_corosync "$nodes_ip"
done

exit 0
