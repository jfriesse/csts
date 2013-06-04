#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that scheduler pause detector works and warning message is logged"

. inc/common.sh

generate_corosync_conf_cb() {
    sed '/^[ \t]*crypto_hash:/a \
    token: 10000'
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

for i in 1 3 6;do
    pause_corosync "$nodes_ip"
    sleep $i
    unpause_corosync "$nodes_ip"
    cat_corosync_log "$nodes_ip" | grep 'Corosync main process was not scheduled for .* ms' && exit 1 || true
done

for i in 9;do
    pause_corosync "$nodes_ip"
    sleep $i
    unpause_corosync "$nodes_ip"
    cat_corosync_log "$nodes_ip" | grep 'Corosync main process was not scheduled for .* ms'
done

exit 0
