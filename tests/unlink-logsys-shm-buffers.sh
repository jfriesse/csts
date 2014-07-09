#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that logsys shm buffers are correctly unlinked on signal or if corosync doesn't enter qb loop"

. inc/common.sh

before_test=`run "$nodes_ip" "ls -1 /dev/shm/"`

configure_corosync "$nodes_ip"

run "$nodes_ip" "corosync -v"
after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`
[ "$before_test" == "$after_test" ]

run "$nodes_ip" "corosync -V" && exit 1 || true
after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`
[ "$before_test" == "$after_test" ]

start_corosync "$nodes_ip"
abrt_corosync "$nodes_ip"
sleep 5
stop_corosync "$nodes_ip" && exit 1 || true
after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`
[ "$before_test" == "$after_test" ]

run "$nodes_ip" "killall -9 corosync || true; rm -f /var/run/corosync.pid"

exit 0
