#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that logsys shm buffers are correctly unlinked on signal or if corosync doesn't enter qb loop"

. inc/common.sh

get_fdata_chsum() {
    local chsum=0

    if run "$nodes_ip" "[ -e '/var/lib/corosync/fdata' ]";then
        chsum=`run "$nodes_ip" "md5sum '/var/lib/corosync/fdata'"`
    fi

    echo ${chsum%% *}
}

before_test=`run "$nodes_ip" "ls -1 /dev/shm/"`

configure_corosync "$nodes_ip"

run "$nodes_ip" "corosync -v"
after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`
[ "$before_test" == "$after_test" ]

run "$nodes_ip" "corosync -V" && exit 1 || true
after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`
[ "$before_test" == "$after_test" ]

# Compute md5 sum of /var/lib/corosync/fdata if file exists
chsum_start=`get_fdata_chsum`
start_corosync "$nodes_ip"
abrt_corosync "$nodes_ip"
sleep 5
stop_corosync "$nodes_ip" && exit 1 || true
after_test=`run "$nodes_ip" "ls -1 /dev/shm/"`
[ "$before_test" == "$after_test" ]
chsum_end=`get_fdata_chsum`

# Make sure that chsum of fdata at the beginning differs from chsum at the end. This means
# that ether fdata didn't exist and now it does, or it is changed. If chsum didn't existed
# and it still doesn't, chsums are equal and this test fails.
[ "$chsum_start" != "$chsum_end" ]

run "$nodes_ip" "killall -9 corosync || true; rm -f /var/run/corosync.pid"

exit 0
