#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that old blackbox format (without timestamp) replaying works and new format contains timestamp"
test_corover_flatiron_enabled=true
test_corover_needle_enabled=false

. inc/common.sh

exit_trap_end_cb() {
    run "$nodes_ip" "rm -f /var/lib/corosync/fdata"
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

# Check old bb format
cat "../data/fplay-flatiron-no-ts-bb-format.xz" | run "$nodes_ip" "xzcat > /var/lib/corosync/fdata"

run "$nodes_ip" "corosync-fplay"

# Check new bb format
run "$nodes_ip" "corosync-blackbox" | egrep 'time=\[[[:digit:]]{4}-[[:digit:]]{2}-[[:digit:]]{2}'

exit 0
