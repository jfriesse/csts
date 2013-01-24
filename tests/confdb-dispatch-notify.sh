#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that corosync handles debug runtime change"

. inc/common.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*debug: .*$/debug: on/'
}

compile_app "$nodes_ip" "testcpg" "-lcpg"

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

cmap_set "$nodes_ip" "logging.debug" "str" "off"

echo "EXIT" | run_app "$nodes_ip" 'testcpg' > /dev/null
cat_corosync_log "$nodes_ip" | grep 'got procjoin message from cluster node' && exit 1

exit 0
