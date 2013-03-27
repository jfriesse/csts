#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that replaying corrupted-fdata properly fails"
test_corover_flatiron_enabled=true
test_corover_needle_enabled=false

. inc/common.sh

exit_trap_end_cb() {
    run "$nodes_ip" "rm -f /var/lib/corosync/fdata"
}

cat "../data/fplay-corrupted-data-fdata.xz" | run "$nodes_ip" "xzcat > /var/lib/corosync/fdata"

run "$nodes_ip" "corosync-fplay" &>/dev/null &
child_pid=$!

sleep 5

res=0
if kill -0 "$child_pid" &> /dev/null;then
    kill -9 "$child_pid"
    res=1
fi

wait $child_pid || true

exit $res
