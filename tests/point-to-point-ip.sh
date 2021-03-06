#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that point to point address is correctly found"

. inc/common.sh
. inc/tap.sh

bindnetaddr="192.168.211.1"

generate_corosync_conf_cb() {
    sed 's/^[ \t]*bindnetaddr:.*$/    bindnetaddr: '$bindnetaddr'/'
}

exit_trap_end_cb() {
    tap_delete_device "$tap_name"
}

tap_name=`tap_create_device`
[ "$tap_name" != "" ]

run "$nodes_ip" "ifconfig $tap_name $bindnetaddr netmask 255.255.255.255 pointopoint 192.168.211.2"
configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

cat_corosync_log "$nodes_ip" | grep "network interface.*$bindnetaddr.*is now up"

exit 0
