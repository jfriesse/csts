#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that shm buffers are correctly unlinked"

. common.sh

before_test=`run "$nodes_ip" "ls -1 /dev/shm/"`

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

cmap_get "$nodes_ip" "logging.debug"

run_as "$nodes_ip" "nobody" "corosync-cfgtool -s" && exit 1

stop_corosync "$nodes_ip"

after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`

[ "$before_test" == "$after_test" ]

exit 0
