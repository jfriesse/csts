#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that corosync parses numbers correctly and test limits"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

generate_corosync_conf_cb_t1() {
    sed '/^[ \t]*crypto_hash:/a \
    token: -1'
}

generate_corosync_conf_cb_t2() {
    sed '/^[ \t]*crypto_hash:/a \
    token: 4294967295'
}

generate_corosync_conf_cb_t3() {
    sed '/^[ \t]*crypto_hash:/a \
    token: 4294967296'
}

generate_corosync_conf_cb_t4() {
    sed 's/mcastport: 5405/mcastport: -1/'
}

generate_corosync_conf_cb_t5() {
    sed 's/mcastport: 5405/mcastport: 65536/'
}

# -1 is invalid number and corosync should exit
configure_corosync "$nodes_ip" generate_corosync_conf_cb_t1
start_corosync "$nodes_ip" && exit 1 || true
start_corosync "$nodes_ip" 2>&1 | grep -i '.*token.*is expected to be integer'

# 4294967295 (2^32 - 1) is valid number and corosync should start
configure_corosync "$nodes_ip" generate_corosync_conf_cb_t2
start_corosync "$nodes_ip"
# Timeout should be set to 4294967295
[ "`cmap_get \"$nodes_ip\" \"totem.token\"`" == "4294967295" ]
stop_corosync "$nodes_ip"

# 4294967296 (2^32) is not valid number and corosync should exit
configure_corosync "$nodes_ip" generate_corosync_conf_cb_t3
start_corosync "$nodes_ip" && exit 1 || true
start_corosync "$nodes_ip" 2>&1 | grep -i '.*token.*is expected to be integer'

# -1 is invalid number for mcastport
configure_corosync "$nodes_ip" generate_corosync_conf_cb_t4
start_corosync "$nodes_ip" && exit 1 || true
start_corosync "$nodes_ip" 2>&1 | grep -i '.*mcastport.*is expected to be integer'

# 65536 (2^16) is not valid mcast port number and corosync should exit
configure_corosync "$nodes_ip" generate_corosync_conf_cb_t5
start_corosync "$nodes_ip" && exit 1 || true
start_corosync "$nodes_ip" 2>&1 | grep -i '.*mcastport.*is expected to be integer'

exit 0
