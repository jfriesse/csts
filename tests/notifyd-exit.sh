#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that notifyd is exited after corosync exits"

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

run "$nodes_ip" "corosync-notifyd -o -f" &>/dev/null &
pid=$!

sleep 2

stop_corosync "$nodes_ip"

sleep 2

res=0

if kill -0 "$pid" &>/dev/null;then
    pkill -9 -P $pid
    res=1
fi

wait $pid

exit $res
