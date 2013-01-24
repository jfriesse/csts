#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that we are using multicast loop socket instead of rely on multicast loop kernel feature"

. inc/common.sh

exit_trap_end_cb() {
    run "$nodes_ip" "iptables -D INPUT ! -i lo -p udp -j DROP && iptables -D OUTPUT ! -o lo -p udp -j DROP"
}

compile_app "$nodes_ip" "testcpg" "-lcpg"

run "$nodes_ip" "iptables -A INPUT ! -i lo -p udp -j DROP && iptables -A OUTPUT ! -o lo -p udp -j DROP"
configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

echo "EXIT" | run_app "$nodes_ip" 'testcpg' > /dev/null

exit 0
