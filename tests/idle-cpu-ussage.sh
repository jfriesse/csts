#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that cpu ussage of corosync in idle is small"

. inc/common.sh

# This test doesn't work with valgrind, so we will simply turn it off
if $use_valgrind;then
    echo "Test doesn't work with valgrind. Valgrind disabled"
    use_valgrind=false
fi

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

# Because corosync_cpu_used returns cumulative cpu ussage (during whole process
# lifetime), we will wait for a while
sleep 3

cpu_used=`corosync_cpu_used "$nodes_ip"`
[ "${cpu_used%%.*}" -lt 3 ]

exit 0
