#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that outq items are freed on exit of IPC connection."

test_max_runtime=7200

. common.sh

compile_app "$nodes_ip" "free-outq-items-on-exit" "-lcpg"

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

run_app "$nodes" "free-outq-items-on-exit"

mem_used_start=`corosync_mem_used "$nodes_ip"`
for i in `seq 1 2`;do
    run_app "$nodes" "free-outq-items-on-exit"
done
mem_used_end=`corosync_mem_used "$nodes_ip"`

[ $mem_used_end -gt $(($mem_used_start / 10 + $mem_used_start)) ] && exit 1

exit 0
