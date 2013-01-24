#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that corosync can start and correctly bind to ipv6 address"

. inc/common.sh

generate_corosync_conf_cb() {
    local ipv6_addr
    local ipv6_mcast_addr

    ipv6_addr=$(echo `ip addr show dev eth0 | grep inet6` | cut -d ' ' -f 2 | cut -d '/' -f 1)
    ipv6_mcast_addr=`echo $ipv6_addr | sed 's/^[a-f0-9]*:/ff3e:/'`

    sed -e 's/^[ \t]*bindnetaddr:.*$/    bindnetaddr: '"$ipv6_addr/" \
        -e 's/^[ \t]*mcastaddr:.*$/    mcastaddr: '$ipv6_mcast_addr'/' \
        -e '/^[ \t]*version/ a \
nodeid: 1' \
        -e '/^[ \t]*version/ a \
ip_version: ipv6'
}

configure_corosync "$nodes_ip"

start_corosync "$nodes_ip"

exit 0
