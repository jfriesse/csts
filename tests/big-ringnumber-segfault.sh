#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that big ringnumber in config doesn't result in segfault"

. inc/common.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*ringnumber: .*$/ringnumber: 100/'
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip" && exit 1 || res=$?
[ "$res" -gt 127 ] && exit 1 || true

cat_corosync_log "$nodes_ip" | grep "interface ring number .* is bigger then allowed maximum "

exit 0
